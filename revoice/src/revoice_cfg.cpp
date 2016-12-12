#include "precompiled.h"

CRevoiceConfig* g_RevoiceConfig;

bool Revoice_Cfg_LoadDefault()
{
	CRevoiceConfig* cfg = CRevoiceConfig::load(REVOICE_CFG_FILE);

	if (!cfg)
		return false;

	if (g_RevoiceConfig)
		delete g_RevoiceConfig;

	g_RevoiceConfig = cfg;

	return true;
}

CRevoiceConfig* CRevoiceConfig::load(const char* fname)
{
	char namebuf[MAX_PATH];
	char gamedir[MAX_PATH];

	sprintf(namebuf, "./%s", fname);

	FILE *fl = fopen(namebuf, "r");

	if (fl == NULL)
	{
		g_engfuncs.pfnGetGameDir(gamedir);
		sprintf(namebuf, "./%s/%s", gamedir, fname);

		fl = fopen(namebuf, "r");

		if (fl == NULL) {
			LCPrintf(true, "Failed to load config: can't find %s in server root or gamedir\n", fname);
			return NULL;
		}
	}

	char linebuf[2048];
	int cline = 0;
	CRevoiceConfig* cfg = createDefault();

	while (fgets(linebuf, sizeof(linebuf), fl))
	{
		cline++;

		char* l = trimbuf(linebuf);

		if (l[0] == '\0' || l[0] == '#')
			continue;

		char* valSeparator = strchr(l, '=');

		if (valSeparator == NULL) {
			LCPrintf(true, "Config line parsing failed: missed '=' on line %d\n", cline);
		}

		*(valSeparator++) = 0;

		char* param = trimbuf(l);
		char* value = trimbuf(valSeparator);

		if (!cfg->parseCfgParam(param, value)) {
			LCPrintf(true, "Config line parsing failed: unknown parameter '%s' at line %d\n", param, cline);
		}
	}

	if (fl)
		fclose(fl);

	return cfg;
}

CRevoiceConfig* CRevoiceConfig::createDefault()
{
	CRevoiceConfig* cfg = new CRevoiceConfig();

	cfg->m_LogMode = rl_console | rl_logfile;

	return cfg;
}

bool CRevoiceConfig::parseCfgParam(const char* param, const char* value)
{

#define REV_CFG_PARSE_INT(paramName, field, _type, minVal, maxVal)			\
	if (!strcasecmp(paramName, param)) {									\
		int i = atoi(value);												\
		if (i < minVal || i > maxVal) {										\
			LCPrintf(true, "Invalid %s value '%s'\n", param, value);		\
			return false;													\
		}																	\
		field = (_type) i;													\
		return true;														\
	}

#define REV_CFG_PARSE_IP(paramName, field)									\
	if (!strcasecmp(paramName, param)) {									\
		field = inet_addr(value);											\
		return true;														\
	}

#define REV_CFG_PARSE_BOOL(paramName, field)								\
	if (!strcasecmp(paramName, param)) {									\
		int i = atoi(value);												\
		if (i < 0 || i > 1) {												\
			LCPrintf(true, "Invalid %s value '%s'\n", param, value);		\
			return false;													\
		}																	\
		field = i ? true : false;											\
		return true;														\
	}

#define REV_CFG_PARSE_STR(paramName, field)									\
	if (!strcasecmp(paramName, param)) {									\
		strncpy(field, value, ARRAYSIZE(field) - 1);						\
		field[ARRAYSIZE(field) - 1] = 0;									\
		return true;														\
	}

	REV_CFG_PARSE_INT("LoggingMode", m_LogMode, int, rl_none, (rl_console | rl_logfile));

	LCPrintf(true, " Config line parsing failed: unknown parameter '%s'\n", param);

	return false;
}
