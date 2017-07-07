#include "precompiled.h"

char g_ExecConfigCmd[MAX_PATH];
const char REVOICE_CFG_FILE[] = "revoice.cfg";

void Revoice_Exec_Config()
{
	if (!g_ExecConfigCmd[0]) {
		return;
	}

	g_engfuncs.pfnServerCommand(g_ExecConfigCmd);
	g_engfuncs.pfnServerExecute();
}

bool Revoice_Init_Config()
{
	const char *pszGameDir = GET_GAME_INFO(PLID, GINFO_GAMEDIR);
	const char *pszPluginDir = GET_PLUGIN_PATH(PLID);

	char szRelativePath[MAX_PATH];
	strncpy(szRelativePath, &pszPluginDir[strlen(pszGameDir) + 1], sizeof(szRelativePath) - 1);
	szRelativePath[sizeof(szRelativePath) - 1] = '\0';
	NormalizePath(szRelativePath);

	char *pos = strrchr(szRelativePath, '/');
	if (pos) {
		*(pos + 1) = '\0';
	}

	snprintf(g_ExecConfigCmd, sizeof(g_ExecConfigCmd), "exec \"%s%s\"\n", szRelativePath, REVOICE_CFG_FILE);
	return true;
}

cvar_t g_cv_rev_hltv_codec    = { "REV_HltvCodec", "opus", 0, 0.0f, nullptr };
cvar_t g_cv_rev_default_codec = { "REV_DefaultCodec", "speex", 0, 0.0f, nullptr };
cvar_t g_cv_rev_version       = { "revoice_version", APP_VERSION, FCVAR_SERVER, 0.0f, nullptr };

cvar_t *g_pcv_rev_hltv_codec    = nullptr;
cvar_t *g_pcv_rev_default_codec = nullptr;
cvar_t *g_pcv_sv_voiceenable    = nullptr;

void Revoice_Init_Cvars()
{
	g_engfuncs.pfnAddServerCommand("rev", Revoice_Cmds_Handler);

	g_engfuncs.pfnCvar_RegisterVariable(&g_cv_rev_version);
	g_engfuncs.pfnCvar_RegisterVariable(&g_cv_rev_hltv_codec);
	g_engfuncs.pfnCvar_RegisterVariable(&g_cv_rev_default_codec);

	g_pcv_sv_voiceenable = g_engfuncs.pfnCVarGetPointer("sv_voiceenable");
	g_pcv_rev_hltv_codec = g_engfuncs.pfnCVarGetPointer(g_cv_rev_hltv_codec.name);
	g_pcv_rev_default_codec = g_engfuncs.pfnCVarGetPointer(g_cv_rev_default_codec.name);
}

REVCmds g_revoice_cmds[] = {
	{ "version", Cmd_REV_Version }
};

void Revoice_Cmds_Handler()
{
	const char *pcmd = CMD_ARGV(1);
	for (auto& cmds : g_revoice_cmds)
	{
		if (_stricmp(cmds.name, pcmd) == 0 && cmds.func) {
			cmds.func();
		}
	}
}

void Cmd_REV_Version()
{
	// print version
	g_engfuncs.pfnServerPrint("Revoice version: " APP_VERSION "\n");
	g_engfuncs.pfnServerPrint("Build date: " APP_COMMIT_TIME " " APP_COMMIT_DATE "\n");
	g_engfuncs.pfnServerPrint("Build from: " APP_COMMIT_URL APP_COMMIT_SHA "\n");
}
