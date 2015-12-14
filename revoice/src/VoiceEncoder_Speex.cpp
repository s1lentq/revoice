#include "precompiled.h"

size_t ENCODED_FRAME_SIZE[] = { 0x6u, 0x6u, 0xFu, 0xFu, 0x14u, 0x14u, 0x1Cu, 0x1Cu, 0x26u, 0x26u, 0x26u };

/* <d91> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:70 */
/*IBaseInterface *CreateSpeexVoiceCodec(void)
{
	IFrameEncoder *pEncoder = (IFrameEncoder *)new VoiceEncoder_Speex;
	return (IBaseInterface *)CreateVoiceCodec_Frame(pEncoder);
}*/

/* <fb3> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:82 */
VoiceEncoder_Speex::VoiceEncoder_Speex()
{
	m_EncoderState = NULL;
	m_DecoderState = NULL;
	m_Quality = 0;
}

/* <e20> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:89 */
VoiceEncoder_Speex::~VoiceEncoder_Speex()
{
	TermStates();
}

/* <efd> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:94 */
bool VoiceEncoder_Speex::Init(int quality, int &rawFrameSize, int &encodedFrameSize)
{
	int postfilter;
	int samplerate;

	if (!InitStates())
		return false;

	rawFrameSize = 320;

	switch (quality) {
	case 2:
		m_Quality = 2;
		break;
	case 3:
		m_Quality = 4;
		break;
	case 4:
		m_Quality = 6;
		break;
	case 5:
		m_Quality = 8;
		break;
	default:
		m_Quality = 0;
		break;
	}

	encodedFrameSize = ENCODED_FRAME_SIZE[m_Quality];

	speex_encoder_ctl(m_EncoderState, SPEEX_SET_QUALITY, &m_Quality);
	//speex_decoder_ctl(m_DecoderState, SPEEX_SET_QUALITY, &m_Quality);

	postfilter = 1;
	speex_decoder_ctl(m_DecoderState, SPEEX_SET_ENH, &postfilter);

	samplerate = 8000;
	speex_decoder_ctl(m_DecoderState, SPEEX_SET_SAMPLING_RATE, &samplerate);
	speex_encoder_ctl(m_EncoderState, SPEEX_SET_SAMPLING_RATE, &samplerate);

	return true;
}

/* <c06> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:127 */
void VoiceEncoder_Speex::Release()
{
	delete this;
}

/* <ce7> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:132 */
void VoiceEncoder_Speex::EncodeFrame(const char *pUncompressedBytes, char *pCompressed)
{
	float input[160];
	int16 *in = (int16 *)pUncompressedBytes;

	for (int i = 0; i < ARRAYSIZE(input); i++, in++) {
		input[i] = *in;
	}

	speex_bits_reset(&m_Bits);
	speex_encode(m_EncoderState, input, &m_Bits);
	speex_bits_write(&m_Bits, pCompressed, ENCODED_FRAME_SIZE[m_Quality]);
}

/* <c53> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:156 */
void VoiceEncoder_Speex::DecodeFrame(const char *pCompressed, char *pDecompressedBytes)
{
	float output[160];
	int16 *out = (int16 *)pDecompressedBytes;

	speex_bits_read_from(&m_Bits, (char *)pCompressed, ENCODED_FRAME_SIZE[m_Quality]);
	speex_decode(m_DecoderState, &m_Bits, output);

	for (int i = 0; i < ARRAYSIZE(output); i++, out++) {
		*out = (int)output[i];
	}
}

/* <c2c> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:175 */
bool VoiceEncoder_Speex::ResetState()
{
	speex_encoder_ctl(m_EncoderState, SPEEX_RESET_STATE, 0);
	speex_decoder_ctl(m_DecoderState, SPEEX_RESET_STATE, 0);

	return true;
}

/* <fd4> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:182 */
bool VoiceEncoder_Speex::InitStates()
{
	speex_bits_init(&m_Bits);
	m_EncoderState = speex_encoder_init(&speex_nb_mode);
	m_DecoderState = speex_decoder_init(&speex_nb_mode);
	return (m_EncoderState != NULL && m_DecoderState != NULL);
}

/* <ff6> ../engine/voice_codecs/speex/VoiceEncoder_Speex.cpp:193 */
void VoiceEncoder_Speex::TermStates()
{
	if (m_EncoderState != NULL)
	{
		speex_encoder_destroy(m_EncoderState);
		m_EncoderState = NULL;
	}

	if (m_DecoderState != NULL)
	{
		speex_encoder_destroy(m_DecoderState);
		m_DecoderState = NULL;
	}

	speex_bits_destroy(&m_Bits);
}

//EXPOSE_INTERFACE_FN(CreateSpeexVoiceCodec, VoiceEncoder_Speex, "voice_speex");
