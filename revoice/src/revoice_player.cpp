
#include "precompiled.h"

CRevoicePlayer g_Players[MAX_PLAYERS];

CRevoicePlayer::CRevoicePlayer() {
	m_CodecType = vct_none;
	m_SpeexCodec = new VoiceCodec_Frame(new VoiceEncoder_Speex());
	m_SilkCodec = new CSteamP2PCodec(new VoiceEncoder_Silk());

	m_SpeexCodec->Init(SPEEX_VOICE_QUALITY);
	m_SilkCodec->Init(SILK_VOICE_QUALITY);

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
	m_VoiceRate = 0;
}

void CRevoicePlayer::OnDisconnected() {
	m_Protocol = 0;
	m_CodecType = vct_none;
	m_VoiceRate = 0;
}

void CRevoicePlayer::OnConnectedRestart(edict_t *pEntity) {
	IGameClient* cl = g_RehldsFuncs->GetHostClient();
	int protocol = g_ReunionApi->GetClientProtocol(cl->GetId());

	if (protocol == 47 || protocol == 48)
	{
		OnConnected(protocol);
		InitVoice(vct_speex);
		if (protocol == 48)
		{
			g_engfuncs.pfnQueryClientCvarValue2(pEntity, "sv_version", 0);
		}
	}
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

void CRevoicePlayer::SetLastVoiceTime(double time)
{
	UpdateVoiceRate(time - m_RehldsClient->GetLastVoiceTime());
	m_RehldsClient->SetLastVoiceTime(time);
}

void CRevoicePlayer::UpdateVoiceRate(double delta)
{
	if (m_VoiceRate)
	{
		switch (m_CodecType)
		{
		case vct_silk:
			m_VoiceRate -= int(delta * MAX_SILK_VOICE_RATE) + MAX_SILK_DATA_LEN;
			break;

		case vct_speex:
			m_VoiceRate -= int(delta * MAX_SPEEX_VOICE_RATE) + MAX_SPEEX_DATA_LEN;
			break;

		default:
			;
		}

		if (m_VoiceRate < 0)
			m_VoiceRate = 0;
	}
}

void CRevoicePlayer::IncreaseVoiceRate(int dataLength)
{
	m_VoiceRate += dataLength;
}

int CRevoicePlayer::GetVoiceRate()
{
	return m_VoiceRate;
}