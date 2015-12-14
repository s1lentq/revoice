#include "precompiled.h"

CSteamP2PCodec::CSteamP2PCodec(IVoiceCodec* backend) {
	m_BackendCodec = backend;
}

bool CSteamP2PCodec::Init(int quality) {
	return m_BackendCodec->Init(quality);
}

void CSteamP2PCodec::Release() {
	m_BackendCodec->Release();
	delete this;
}

bool CSteamP2PCodec::ResetState() {
	return m_BackendCodec->ResetState();
}

int CSteamP2PCodec::StreamDecode(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes) const {
	const char* maxReadPos = pCompressed + compressedBytes;
	const char* readPos = pCompressed;
	while (readPos < maxReadPos) {
		uint8 opcode = *(uint8*)readPos;
		readPos++;

		switch (opcode) {
			case 0xB: //Set sampling rate
				if (readPos + 2 > maxReadPos) {
					return 0;
				}
				readPos += 2;
				break;

			case 0x04: //Voice payoad
			{
				if (readPos + 2 > maxReadPos) {
					return 0;
				}
				uint16 len = *(uint16*)readPos;
				readPos += 2;

				if (readPos + len > maxReadPos) {
					return 0;
				}
				
				int decompressedLen = m_BackendCodec->Decompress(readPos, len, pUncompressed, maxUncompressedBytes);
				return decompressedLen;
			}

			default: //Invalid or unknown opcode
				return 0; 
		}
	}

	return 0; // no voice payload in the stream
}

int CSteamP2PCodec::StreamEncode(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal) const {
	char* writePos = pCompressed;
	if (maxCompressedBytes < 10) { // no room
		return 0;
	}

	*(writePos++) = 0x0B; //set sampling rate
	*(uint16*)writePos = 16000;
	writePos += 2;

	*(writePos++) = 0x04; //voice payload

	int compressRes = m_BackendCodec->Compress(pUncompressedBytes, nSamples, writePos + 2, maxCompressedBytes - (1 + 2 + 1 + 2), bFinal);
	if (compressRes == 0) {
		return 0;
	}

	*(uint16*)writePos = compressRes;
	writePos += 2;
	writePos += compressRes;

	return writePos - pCompressed;
}

int CSteamP2PCodec::Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes) {
	if (compressedBytes < 12) {
		return 0;
	}

	uint32 computedChecksum = crc32(pCompressed, compressedBytes - 4);
	uint32 wireChecksum = *(uint32*)(pCompressed + compressedBytes - 4);
	if (computedChecksum != wireChecksum) {
		return 0;
	}

	return StreamDecode(pCompressed + 8, compressedBytes - 12, pUncompressed, maxUncompressedBytes);
}

int CSteamP2PCodec::Compress(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal) {
	if (maxCompressedBytes < 12) { //no room
		return 0;
	}

	char* writePos = pCompressed;
	*(uint32*)writePos = 0x00000011; //steamid (low part)
	writePos += 4;

	*(uint32*)writePos = 0x01100001; //steamid (high part)
	writePos += 4;

	int encodeRes = StreamEncode(pUncompressedBytes, nSamples, writePos, maxCompressedBytes - 12, bFinal);
	if (encodeRes <= 0) {
		return 0;
	}
	writePos += encodeRes;

	uint32 cksum = crc32(pCompressed, writePos - pCompressed);
	*(uint32*)writePos = cksum;
	writePos += 4;

	return writePos - pCompressed;
}
