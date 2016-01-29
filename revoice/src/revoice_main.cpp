
#include "precompiled.h"

cvar_t* pcv_sv_voiceenable = NULL;

void Rehlds_ClientConnected_Hook(IRehldsHook_ClientConnected* chain, IGameClient* cl) {
	int protocol = g_ReunionApi->GetClientProtocol(cl->GetId());

	if (protocol == 47 || protocol == 48) {
		CRevoicePlayer* plr = GetPlayerByClientPtr(cl);
		plr->OnConnected(protocol);

		// default codec
		plr->InitVoice(vct_speex);

		//for p48 we will query sv_version cvar value later in mm_ClientConnect
	}

	chain->callNext(cl);
}

qboolean mm_ClientConnect(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) {
	CRevoicePlayer* plr = GetPlayerByEdict(pEntity);
	int protocol = plr->GetProtocol();

	if (protocol == 0)
	{
		plr->OnConnectedRestart(pEntity);
	}
	else if (protocol == 48)
	{
		g_engfuncs.pfnQueryClientCvarValue2(pEntity, "sv_version", 0);
	}

	RETURN_META_VALUE(MRES_IGNORED, TRUE);
}

void mm_ClientDisconnect(edict_t *pEntity)
{
	CRevoicePlayer* plr = GetPlayerByEdict(pEntity);
	plr->OnDisconnected();
	RETURN_META(MRES_IGNORED);
}

void mm_CvarValue2(const edict_t *pEnt, int requestID, const char *cvarName, const char *cvarValue) {
	if (!strcmp("sv_version", cvarName)) {
		// ] sv_version
		// "sv_version" is "1.1.2.1/2.0.0.0,48,4554"

		const char* lastSeparator = strrchr(cvarValue, ',');
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

int TranscodeVoice(const char* srcBuf, int srcBufLen, IVoiceCodec* srcCodec, IVoiceCodec* dstCodec, char* dstBuf, int dstBufSize) {
	char decodedBuf[32768];
	int numDecodedSamples = srcCodec->Decompress(srcBuf, srcBufLen, decodedBuf, sizeof(decodedBuf));
	if (numDecodedSamples <= 0) {
		return 0;
	}

	int compressedSize = dstCodec->Compress(decodedBuf, numDecodedSamples, dstBuf, dstBufSize, false);
	if (compressedSize <= 0) {
		return 0;
	}

	/*
	int numDecodedSamples2 = dstCodec->Decompress(dstBuf, compressedSize, decodedBuf, sizeof(decodedBuf));
	if (numDecodedSamples2 <= 0) {
		return compressedSize;
	}

	FILE* rawSndFile = fopen("d:\\revoice_raw.snd", "ab");
	if (rawSndFile) {
		fwrite(decodedBuf, 2, numDecodedSamples2, rawSndFile);
		fclose(rawSndFile);
	}
	*/

	return compressedSize;
}

void SV_ParseVoiceData_emu(IGameClient* cl) {
	if (pcv_sv_voiceenable->value == 0.0f) {
		return;
	}

	char chReceived[4096];
	unsigned int nDataLength = g_RehldsFuncs->MSG_ReadShort();

	if (nDataLength > sizeof(chReceived)) {
		g_RehldsFuncs->DropClient(cl, FALSE, "Invalid voice data\n");
		return;
	}

	g_RehldsFuncs->MSG_ReadBuf(nDataLength, chReceived);

	CRevoicePlayer* srcPlayer = GetPlayerByClientPtr(cl);
	srcPlayer->SetLastVoiceTime(g_RehldsSv->GetTime());
	srcPlayer->IncreaseVoiceRate(nDataLength);

	char transcodedBuf[4096];
	char* speexData; int speexDataLen;
	char* silkData; int silkDataLen;

	switch (srcPlayer->GetCodecType()) {
		case vct_silk:
		{
			if (nDataLength > MAX_SILK_DATA_LEN || srcPlayer->GetVoiceRate() > MAX_SILK_VOICE_RATE)
				return;

			silkData = chReceived; silkDataLen = nDataLength;
			speexData = transcodedBuf;
			speexDataLen = TranscodeVoice(silkData, silkDataLen, srcPlayer->GetSilkCodec(), srcPlayer->GetSpeexCodec(), transcodedBuf, sizeof(transcodedBuf));
			break;
		}

		case vct_speex:
			if (nDataLength > MAX_SPEEX_DATA_LEN || srcPlayer->GetVoiceRate() > MAX_SPEEX_VOICE_RATE)
				return;

			speexData = chReceived; speexDataLen = nDataLength;
			silkData = transcodedBuf;
			silkDataLen = TranscodeVoice(speexData, speexDataLen, srcPlayer->GetSpeexCodec(), srcPlayer->GetSilkCodec(), transcodedBuf, sizeof(transcodedBuf));
			break;

		default:
			return;
	}

	int maxclients = g_RehldsSvs->GetMaxClients();

	for (int i = 0; i < maxclients; i++) {
		CRevoicePlayer* dstPlayer = &g_Players[i];
		IGameClient* dstClient = dstPlayer->GetClient();

		if (!((1 << i) & cl->GetVoiceStream(0)) && dstPlayer != srcPlayer)
			continue;

		if (!dstClient->IsActive() && !dstClient->IsConnected() && dstPlayer != srcPlayer)
			continue;

		char* sendBuf; int nSendLen;

		switch (dstPlayer->GetCodecType()) {
			case vct_silk:
				sendBuf = silkData; nSendLen = silkDataLen;
				break;

			case vct_speex:
				sendBuf = speexData; nSendLen = speexDataLen;
				break;

			default:
				sendBuf = NULL; nSendLen = 0;
				break;
		}

		if (sendBuf == NULL || nSendLen == 0)
			continue;

		if (dstPlayer == srcPlayer && !dstClient->GetLoopback())
			nSendLen = 0;

		sizebuf_t* dstDatagram = dstClient->GetDatagram();
		if (dstDatagram->cursize + nSendLen + 6 < dstDatagram->maxsize) {
			g_RehldsFuncs->MSG_WriteByte(dstDatagram, svc_voicedata); //svc_voicedata
			g_RehldsFuncs->MSG_WriteByte(dstDatagram, cl->GetId());
			g_RehldsFuncs->MSG_WriteShort(dstDatagram, nSendLen);
			g_RehldsFuncs->MSG_WriteBuf(dstDatagram, nSendLen, sendBuf);
		}
	}
}

void Rehlds_HandleNetCommand(IRehldsHook_HandleNetCommand* chain, IGameClient* cl, int8 opcode) {
	if (opcode == 8) { //clc_voicedata
		SV_ParseVoiceData_emu(cl);
		return;
	}

	chain->callNext(cl, opcode);
}

void SV_WriteVoiceCodec_hooked(IRehldsHook_SV_WriteVoiceCodec* chain, sizebuf_t* sb) {
	IGameClient* cl = g_RehldsFuncs->GetHostClient();
	CRevoicePlayer* plr = GetPlayerByClientPtr(cl);

	switch (plr->GetCodecType()) {
	case vct_silk:
		g_RehldsFuncs->MSG_WriteByte(sb, svc_voiceinit); //svc_voiceinit
		g_RehldsFuncs->MSG_WriteString(sb, ""); //codec id
		g_RehldsFuncs->MSG_WriteByte(sb, 0); //quality
		break;

	case vct_speex:
		g_RehldsFuncs->MSG_WriteByte(sb, svc_voiceinit); //svc_voiceinit
		g_RehldsFuncs->MSG_WriteString(sb, "voice_speex"); //codec id
		g_RehldsFuncs->MSG_WriteByte(sb, 5); //quality
		break;

	default:
		//LCPrintf(true, "SV_WriteVoiceCodec() called on client(%d) with unknown voice codec\n", cl->GetId());
		
		// set default codec
		plr->InitVoice(vct_speex);
		g_RehldsFuncs->MSG_WriteByte(sb, svc_voiceinit); //svc_voiceinit
		g_RehldsFuncs->MSG_WriteString(sb, "voice_speex"); //codec id
		g_RehldsFuncs->MSG_WriteByte(sb, 5); //quality
		break;

	}
}

bool Revoice_Main_Init() {
	g_RehldsHookchains->ClientConnected()->registerHook(Rehlds_ClientConnected_Hook);
	g_RehldsHookchains->HandleNetCommand()->registerHook(Rehlds_HandleNetCommand);
	g_RehldsHookchains->SV_WriteVoiceCodec()->registerHook(SV_WriteVoiceCodec_hooked);
	pcv_sv_voiceenable = g_engfuncs.pfnCVarGetPointer("sv_voiceenable");

	return true;
}