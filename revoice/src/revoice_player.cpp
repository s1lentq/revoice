#include "precompiled.h"

CRevoicePlayer g_Players[MAX_PLAYERS];

CRevoicePlayer::CRevoicePlayer()
{
	m_CodecType = vct_none;
	m_SpeexCodec = new VoiceCodec_Frame(new VoiceEncoder_Speex());
	m_SilkCodec = new CSteamP2PCodec(new VoiceEncoder_Silk());
	m_OpusCodec = new CSteamP2PCodec(new VoiceEncoder_Opus());

	m_SpeexCodec->Init(SPEEX_VOICE_QUALITY);
	m_SilkCodec->Init(SILK_VOICE_QUALITY);
	m_OpusCodec->Init(OPUS_VOICE_QUALITY);

	m_RehldsClient = NULL;
	m_Protocol = 0;
	m_Connected = false;
}

void CRevoicePlayer::Initialize(IGameClient* cl)
{
	m_RehldsClient = cl;
}

void CRevoicePlayer::OnConnected()
{
	// already connected, suppose now there is a change of level?
	if (m_Connected) {
		m_VoiceRate = 0;
		return;
	}

	int protocol = g_ReunionApi->GetClientProtocol(m_RehldsClient->GetId());

	if (protocol != 47 && protocol != 48) {
		return;
	}

	// reset codec state
	m_SilkCodec->ResetState();
	m_OpusCodec->ResetState();
	m_SpeexCodec->ResetState();

	// default codec
	m_CodecType = vct_speex;
	m_VoiceRate = 0;
	m_Connected = true;
	m_RequestId = MAKE_REQUESTID(PLID);
	m_Protocol = protocol;

	if (m_Protocol == 48) {
		g_engfuncs.pfnQueryClientCvarValue2(m_RehldsClient->GetEdict(), "sv_version", m_RequestId);
	}
}

void CRevoicePlayer::OnDisconnected()
{
	m_Connected = false;
	m_Protocol = 0;
	m_CodecType = vct_none;
	m_VoiceRate = 0;
	m_RequestId = 0;
}

void Revoice_Init_Players()
{
	int maxclients = g_RehldsSvs->GetMaxClients();

	for (int i = 0; i < maxclients; i++) {
		g_Players[i].Initialize(g_RehldsSvs->GetClient(i));
	}
}

CRevoicePlayer *GetPlayerByClientPtr(IGameClient *cl)
{
	return &g_Players[ cl->GetId() ];
}

CRevoicePlayer *GetPlayerByEdict(const edict_t *ed)
{
	int clientId = g_engfuncs.pfnIndexOfEdict(ed) - 1;

	if (clientId < 0 || clientId >= g_RehldsSvs->GetMaxClients()) {
		util_syserror("Invalid player edict id=%d\n", clientId);
	}

	return &g_Players[ clientId ];
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

			case vct_opus:
				m_VoiceRate -= int(delta * MAX_OPUS_VOICE_RATE) + MAX_OPUS_DATA_LEN;
				break;

			case vct_speex:
				m_VoiceRate -= int(delta * MAX_SPEEX_VOICE_RATE) + MAX_SPEEX_DATA_LEN;
				break;

			default:
				break;
		}

		if (m_VoiceRate < 0)
			m_VoiceRate = 0;
	}
}

void CRevoicePlayer::IncreaseVoiceRate(int dataLength)
{
	m_VoiceRate += dataLength;
}
