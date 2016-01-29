#pragma once

#include "revoice_shared.h"
#include "VoiceEncoder_Silk.h"
#include "SteamP2PCodec.h"
#include "VoiceEncoder_Speex.h"
#include "voice_codec_frame.h"


class CRevoicePlayer {
private:
	IGameClient* m_RehldsClient;
	revoice_codec_type m_CodecType;
	CSteamP2PCodec* m_SilkCodec;
	VoiceCodec_Frame* m_SpeexCodec;
	int m_Protocol;
	int m_VoiceRate;

public:
	CRevoicePlayer();
	void Initialize(IGameClient* cl);
	void OnConnected(int protocol);
	void OnDisconnected();
	void InitVoice(revoice_codec_type codecType);
	void SetLastVoiceTime(double time);
	void UpdateVoiceRate(double delta);
	void IncreaseVoiceRate(int dataLength);
	void OnConnectedRestart(edict_t *pEntity);
	int GetVoiceRate();
	int GetProtocol() const { return m_Protocol; }
	revoice_codec_type GetCodecType() const { return m_CodecType; }
	CSteamP2PCodec* GetSilkCodec() const { return m_SilkCodec; }
	VoiceCodec_Frame* GetSpeexCodec() const { return m_SpeexCodec;  }
	IGameClient* GetClient() const { return m_RehldsClient; }
};

extern CRevoicePlayer g_Players[MAX_PLAYERS];

extern void Revoice_Init_Players();
extern CRevoicePlayer* GetPlayerByClientPtr(IGameClient* cl);
extern CRevoicePlayer* GetPlayerByEdict(const edict_t* ed);
