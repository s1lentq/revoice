#pragma once

#include "IVoiceCodec.h"
#include "iframeencoder.h"
#include "speex.h"

/* <61c> ../engine/voice_codecs/speex/VoiceEncoder_Speex.h:57 */
class VoiceEncoder_Speex: public IFrameEncoder {
protected:
	virtual ~VoiceEncoder_Speex();

public:
	VoiceEncoder_Speex();

	/* <6c8> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:94 */
	bool Init(int quality, int &rawFrameSize, int &encodedFrameSize);

	/* <6ff> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:127 */
	void Release();

	/* <751> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:132 */
	void EncodeFrame(const char *pUncompressedBytes, char *pCompressed);

	/* <723> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:156 */
	void DecodeFrame(const char *pCompressed, char *pDecompressedBytes);

	/* <77f> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:175 */
	bool ResetState();

protected:
	/* <7a7> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:182 */
	bool InitStates();

	/* <7c8> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:193 */
	void TermStates();

private:
	int m_Quality;
	void *m_EncoderState;
	void *m_DecoderState;
	SpeexBits m_Bits;

};

extern IVoiceCodec *CreateVoiceCodec_Frame(IFrameEncoder *pEncoder);
