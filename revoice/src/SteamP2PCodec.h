#pragma once

#include "revoice_shared.h"
#include "IVoiceCodec.h"

class CSteamP2PCodec : public IVoiceCodec {
public:
	CSteamP2PCodec(IVoiceCodec *backend);
	virtual bool Init(int quality);
	virtual void Release();
	virtual int Compress(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal);
	virtual int Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes);
	virtual bool ResetState();

	IVoiceCodec *GetCodec() const { return m_BackendCodec; }

private:
	int StreamDecode(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes) const;
	int StreamEncode(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal) const;

private:
	IVoiceCodec *m_BackendCodec;
};
