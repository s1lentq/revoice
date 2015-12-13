#include "precompiled.h"

/* <1f68> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:23 */
VoiceCodec_Frame::VoiceCodec_Frame(IFrameEncoder *pEncoder)
{
	m_nEncodeBufferSamples = 0;
	m_nRawBytes = m_nRawSamples = m_nEncodedBytes = 0;
	m_pFrameEncoder = pEncoder;
}

/* <2107> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:30 */
VoiceCodec_Frame::~VoiceCodec_Frame()
{
	if (m_pFrameEncoder != NULL) {
		m_pFrameEncoder->Release();
		m_pFrameEncoder = NULL;
	}
}

/* <22bc> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:36 */
bool VoiceCodec_Frame::Init(int quality)
{
	if (m_pFrameEncoder == NULL)
		return false;

	if (m_pFrameEncoder->Init(quality, m_nRawBytes, m_nEncodedBytes)) {
		m_nRawSamples = m_nRawBytes >> 1;
		return true;
	} else {
		m_pFrameEncoder->Release();
		m_pFrameEncoder = NULL;
		return false;
	}
}

/* <2038> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:54 */
void VoiceCodec_Frame::Release()
{
	delete this;
}

/* <21fb> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:59 */
int VoiceCodec_Frame::Compress(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal)
{
	if (m_pFrameEncoder == NULL)
		return 0;

	const int16 *pUncompressed = (const int16*) pUncompressedBytes;

	int nCompressedBytes = 0;
	while ((nSamples + m_nEncodeBufferSamples) >= m_nRawSamples && (maxCompressedBytes - nCompressedBytes) >= m_nEncodedBytes)
	{
		// Get the data block out.
		int16 samples[MAX_FRAMEBUFFER_SAMPLES];
		memcpy(samples, m_EncodeBuffer, m_nEncodeBufferSamples*BYTES_PER_SAMPLE);
		memcpy(&samples[m_nEncodeBufferSamples], pUncompressed, (m_nRawSamples - m_nEncodeBufferSamples) * BYTES_PER_SAMPLE);
		nSamples -= m_nRawSamples - m_nEncodeBufferSamples;
		pUncompressed += m_nRawSamples - m_nEncodeBufferSamples;
		m_nEncodeBufferSamples = 0;
			
		// Compress it.
		m_pFrameEncoder->EncodeFrame((const char*)samples, &pCompressed[nCompressedBytes]);
		nCompressedBytes += m_nEncodedBytes;
	}

	// Store the remaining samples.
	int nNewSamples = _min(nSamples, _min(m_nRawSamples-m_nEncodeBufferSamples, m_nRawSamples));
	if (nNewSamples) {
		memcpy(&m_EncodeBuffer[m_nEncodeBufferSamples], &pUncompressed[nSamples - nNewSamples], nNewSamples*BYTES_PER_SAMPLE);
		m_nEncodeBufferSamples += nNewSamples;
	}

	// If it must get the last data, just pad with zeros..
	if (bFinal && m_nEncodeBufferSamples && (maxCompressedBytes - nCompressedBytes) >= m_nEncodedBytes)
	{
		memset(&m_EncodeBuffer[m_nEncodeBufferSamples], 0, (m_nRawSamples - m_nEncodeBufferSamples) * BYTES_PER_SAMPLE);
		m_pFrameEncoder->EncodeFrame((const char*)m_EncodeBuffer, &pCompressed[nCompressedBytes]);
		nCompressedBytes += m_nEncodedBytes;
		m_nEncodeBufferSamples = 0;
	}

	return nCompressedBytes;
}

/* <205e> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:102 */
int VoiceCodec_Frame::Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes)
{
	if (m_pFrameEncoder == NULL || compressedBytes < m_nEncodedBytes || maxUncompressedBytes < m_nRawBytes)
		return 0;

	int nDecompressedBytes = 0;
	int curCompressedByte = 0;

	while (true)
	{
		m_pFrameEncoder->DecodeFrame(&pCompressed[curCompressedByte], &pUncompressed[nDecompressedBytes]);

		curCompressedByte += m_nEncodedBytes;
		nDecompressedBytes += m_nRawBytes;

		if (compressedBytes - curCompressedByte < m_nEncodedBytes || maxUncompressedBytes - nDecompressedBytes < m_nRawBytes)
			break;
	}

	return nDecompressedBytes / BYTES_PER_SAMPLE;
}

/* <20e1> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:120 */
bool VoiceCodec_Frame::ResetState()
{
	if (m_pFrameEncoder)
		return m_pFrameEncoder->ResetState();
	else
		return false;
}

/* <230a> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:141 */
IVoiceCodec *CreateVoiceCodec_Frame(IFrameEncoder *pEncoder)
{
	return new VoiceCodec_Frame(pEncoder);
}
