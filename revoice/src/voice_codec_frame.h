#pragma once

/* <19b1> ../engine/voice_codecs/speex/../frame_encoder/voice_codec_frame.cpp:18 */
class VoiceCodec_Frame: public IVoiceCodec {
public:
	virtual ~VoiceCodec_Frame();
	VoiceCodec_Frame(IFrameEncoder *pEncoder);

	bool Init(int quality);
	void Release();
	int Compress(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal);
	int Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes);
	bool ResetState();

protected:
	enum { MAX_FRAMEBUFFER_SAMPLES = 1024 };

	short int m_EncodeBuffer[MAX_FRAMEBUFFER_SAMPLES];
	int m_nEncodeBufferSamples;

	IFrameEncoder *m_pFrameEncoder;
	int m_nRawBytes;
	int m_nRawSamples;
	int m_nEncodedBytes;

};/* size: 2072, cachelines: 33, members: 7 */
