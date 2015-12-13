
#include "precompiled.h"

int g_ClientBeingConnected_Protocol = 0;

void SV_ConnectClient_hook(IRehldsHook_SV_ConnectClient* chain) {
	g_ClientBeingConnected_Protocol = atoi(g_engfuncs.pfnCmd_Argv(1));
	chain->callNext();
}

void Rehlds_ClientConnected_Hook(IRehldsHook_ClientConnected* chain, IGameClient* cl) {
	if (g_ClientBeingConnected_Protocol >= 47 && g_ClientBeingConnected_Protocol <= 48) {
		CRevoicePlayer* plr = GetPlayerByClientPtr(cl);
		plr->OnConnected(g_ClientBeingConnected_Protocol);

		if (g_ClientBeingConnected_Protocol == 47) {
			plr->InitVoice(vct_speex);
		} else {
			g_engfuncs.pfnQueryClientCvarValue2(cl->GetEdict(), "sv_version", 0);
		}
	}

	chain->callNext(cl);
}

void mm_CvarValue2(const edict_t *pEnt, int requestID, const char *cvarName, const char *value) {
	if (!strcmp("sv_version", cvarName)) {
		// ] sv_version
		// "sv_version" is "1.1.2.1/2.0.0.0,48,4554"

		const char* lastSeparator = strrchr(cvarName, ',');
		int buildNumber = 0;
		if (lastSeparator) {
			buildNumber = atoi(lastSeparator + 1);
		}

		CRevoicePlayer* plr = GetPlayerByEdict(pEnt);
		if (buildNumber > 4554) {
			plr->InitVoice(vct_silk);
		} else {
			plr->InitVoice(vct_speex);
		}
	}

	RETURN_META(MRES_IGNORED);
}

qboolean mm_ClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) {
	RETURN_META_VALUE(MRES_IGNORED, 1);
}

bool Revoice_Main_Init() {
	g_RehldsHookchains->SV_ConnectClient()->registerHook(SV_ConnectClient_hook);
	g_RehldsHookchains->ClientConnected()->registerHook(Rehlds_ClientConnected_Hook);
	return true;
}