
#include "precompiled.h"

CRevoicePlayer g_Players[MAX_PLAYERS];

CRevoicePlayer::CRevoicePlayer() {
	m_CodecType = vct_none;
	m_SpeexCodec = new VoiceCodec_Frame(new VoiceEncoder_Speex());
	m_SilkCodec = new VoiceEncoder_Silk();

	m_SpeexCodec->Init(5);
	m_SilkCodec->Init(5);

	m_RehldsClient = NULL;
	m_Protocol = 0;
}

void CRevoicePlayer::Initialize(IGameClient* cl) {
	m_RehldsClient = cl;
}

void CRevoicePlayer::InitVoice(revoice_codec_type codecType) {
	m_CodecType = codecType;
	m_SilkCodec->ResetState();
	m_SpeexCodec->ResetState();
}

void CRevoicePlayer::OnConnected(int protocol) {
	m_Protocol = protocol;
	m_CodecType = vct_none;
}	

void Revoice_Init_Players() {
	int maxclients = g_RehldsSvs->GetMaxClients();

	for (int i = 0; i < maxclients; i++) {
		g_Players[i].Initialize(g_RehldsSvs->GetClient(i));
	}
}

CRevoicePlayer* GetPlayerByClientPtr(IGameClient* cl) {
	return &g_Players[cl->GetId()];
}

CRevoicePlayer* GetPlayerByEdict(const edict_t* ed) {
	int clientId = g_engfuncs.pfnIndexOfEdict(ed) - 1;
	if (clientId < 0 || clientId >= g_RehldsSvs->GetMaxClients()) {
		util_syserror("Invalid player edict id=%d\n", clientId);
	}

	return &g_Players[clientId];
}