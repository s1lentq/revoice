#pragma once

#include "revoice_shared.h"
#include "VoiceEncoder_Silk.h"
#include "VoiceEncoder_Speex.h"
#include "voice_codec_frame.h"

class CRevoicePlayer {
private:
	IGameClient* m_RehldsClient;
	revoice_codec_type m_CodecType;
	VoiceEncoder_Silk* m_SilkCodec;
	VoiceCodec_Frame* m_SpeexCodec;
	int m_Protocol;

public:
	CRevoicePlayer();
	void Initialize(IGameClient* cl);
	void OnConnected(int protocol);
	void InitVoice(revoice_codec_type codecType);
	

};

extern CRevoicePlayer g_Players[MAX_PLAYERS];

extern void Revoice_Init_Players();
extern CRevoicePlayer* GetPlayerByClientPtr(IGameClient* cl);
extern CRevoicePlayer* GetPlayerByEdict(const edict_t* ed);
