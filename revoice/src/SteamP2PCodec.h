#pragma once

#include "revoice_shared.h"
#include "VoiceEncoder_Silk.h"

class CSteamP2PCodec : public IVoiceCodec {
public:
	CSteamP2PCodec(VoiceEncoder_Silk* backend);
	virtual bool Init(int quality);
	virtual void Release();
	virtual int Compress(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal);
	virtual int Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes);
	virtual bool ResetState();

	VoiceEncoder_Silk* GetSilkCodec() const { return m_BackendCodec; }

private:
	int StreamDecode(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes) const;
	int StreamEncode(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal) const;
private:
	VoiceEncoder_Silk* m_BackendCodec;

	enum P2P_PayloadType_e:uint8
	{
		P2P_Silence, //Number of empty samples, which should be set to NULL.
		P2P_UnknownCodec, /*
								Not used for now. I suppose, it was used for Miles voice data. 
								Returns "No decoder available, abandoning voice\n" in SteamClient.
								*/
		P2P_Speex, /*
					Not used for now. "No speex decoder available, abandoning voice\n" in SteamClient.
					In fact, k_EVoiceResultUnsupportedCodec(7) should be returned after it.. but who cares
					*/
		P2P_Raw,  //Can be used in theory.    
		P2P_Silk, 
		P2P_Unknown2Bytes = 0xA, //Can't understand what for is it, but it's exists in SteamClient.
		P2P_SamplingRate
	};
};