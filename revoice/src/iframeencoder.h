#pragma once

/* <7e2> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:15 */
class IFrameEncoder {
protected:
	virtual ~IFrameEncoder() {};

public:
	// quality is in [0..10]
	/* <855> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:23 */
	virtual bool Init(int quality, int &rawFrameSize, int &encodedFrameSize) = 0;

	/* <88c> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:25 */
	virtual void Release() = 0;

	/* <8b0> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:29 */
	virtual void EncodeFrame(const char *pUncompressedBytes, char *pCompressed) = 0;

	/* <8de> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:33 */
	virtual void DecodeFrame(const char *pCompressed, char *pDecompressedBytes) = 0;

	/* <90c> ../engine/voice_codecs/speex/../frame_encoder/iframeencoder.h:36 */
	virtual bool ResetState() = 0;
};
