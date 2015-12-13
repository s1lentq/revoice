
#include "revoice_shared.h"
#include "VoiceEncoder_Silk.h"

class CRevoicePlayer {
private:
	revoice_codec_type m_CodecType;
	VoiceEncoder_Silk* m_SilkCodec;
	IVoiceCodec* m_SpeexCodec;
};