#include "precompiled.h"

cvar_t cv_revoice_version = {"revoice_version", APP_VERSION_STRD, FCVAR_SERVER|FCVAR_EXTDLL, 0, NULL};

cvar_t* pcv_revoice_version;
cvar_t* pcv_mp_logecho;
char logstring[2048];

void LCPrintf(bool critical, const char *fmt, ...) {
	va_list			argptr;
	const int		prefixlen = 11; //sizeof(LOG_PREFIX) - 1;

	va_start(argptr, fmt);
	vsnprintf(logstring + prefixlen, sizeof(logstring) - prefixlen, fmt, argptr);
	va_end(argptr);

	bool bNeedWriteInConsole = critical || (g_RevoiceConfig && g_RevoiceConfig->hasLogMode(rl_console));
	bool bNeedWriteInLog = critical || (g_RevoiceConfig && g_RevoiceConfig->hasLogMode(rl_logfile));

	if (bNeedWriteInConsole && bNeedWriteInLog && g_RehldsSvs && g_RehldsSvs->IsLogActive())
	{
		if (pcv_mp_logecho && pcv_mp_logecho->value != 0.0)
			bNeedWriteInConsole = false;
	}

	if (bNeedWriteInConsole)
		SERVER_PRINT(logstring);

	if (bNeedWriteInLog)
		ALERT(at_logged, logstring);
}

bool Revoice_Utils_Init() {
	g_engfuncs.pfnCvar_RegisterVariable(&cv_revoice_version);

	pcv_revoice_version = g_engfuncs.pfnCVarGetPointer(cv_revoice_version.name);
	pcv_mp_logecho = g_engfuncs.pfnCVarGetPointer("mp_logecho");

	strcpy(logstring, LOG_PREFIX);
	return true;
}

char* trimbuf(char *str) {
	char *ibuf = str;
	int i = 0;

	if (str == NULL) return NULL;
	for (ibuf = str; *ibuf && (byte)(*ibuf) < (byte)0x80 && isspace(*ibuf); ++ibuf)
		;

	i = strlen(ibuf);
	if (str != ibuf)
		memmove(str, ibuf, i);

	str[i] = 0;
	while (--i >= 0) {
		if (!isspace(ibuf[i]))
			break;
	}
	ibuf[++i] = 0;
	return str;
}

uint32 crc32(const void* buf, unsigned int bufLen) {
	CRC32_t hCrc;
	g_engfuncs.pfnCRC32_Init(&hCrc);
	g_engfuncs.pfnCRC32_ProcessBuffer(&hCrc, (void*)buf, bufLen);
	hCrc = g_engfuncs.pfnCRC32_Final(hCrc);
	return hCrc;
}


void util_syserror(const char* fmt, ...)
{
	va_list	argptr;
	char buf[4096];
	va_start(argptr, fmt);
	vsnprintf(buf, ARRAYSIZE(buf) - 1, fmt, argptr);
	buf[ARRAYSIZE(buf) - 1] = 0;
	va_end(argptr);

	LCPrintf(true, "ERROR: %s", buf);
	*(int *)0 = 0;
}
