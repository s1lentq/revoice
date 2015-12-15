#include "precompiled.h"


static_assert(sizeof(uint16) == 2,"Sizeof(uint16) should be \"2\". Check your platform");


CSteamP2PCodec::CSteamP2PCodec(VoiceEncoder_Silk* backend) {
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


/*
	Packet description:
	___START___
	uint64 - >Speaker SteamID
	//Payload data:
		byte: PayloadType
		0: Silence samples
			uint16->Number of silence samples; (memset(&Data,0,sizeof(Sample)*Num);
		1-4: VoiceData:
			DataType:
				1:Unknown codec,returns 0. ("No decoder available, abandoning voice\n");
				2:Speex codec. Returns 0 too ("No speex decoder available, abandoning voice\n"); //returns 7 (k_EVoiceResultUnsupportedCodec).
				3:Uncompressed data (memmove, всё такое)
				4: Silk codec.
			uint16: NumOfDataBytes;
			while(NumOfDataBytes)
			{
				EncodedBytes:int16(-1:EndOfPacket, 0:Silence)
				Decompress...
			}

		10:Unknown
			Read 2 bytes, which not used.
		11: Sample rate
			uint16: SampleRate. (Looks like packet sample rate).
			DecompressedFrameSz=20*SampleRate/1000;
		default:
			return 5(k_EVoiceResultDataCorrupted);
	4 bytes - CRC32 of all packet, except this 4 bytes; Returns 5 (k_EVoiceResultDataCorrupted), if not valid;
	__END___
*/
	
int CSteamP2PCodec::StreamDecode(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes) const {
	const char* maxReadPos = pCompressed + compressedBytes;
	const char* readPos = pCompressed;
	qboolean DecodingError = FALSE;  
	int NumDecompressedBytes = 0; 

	auto IsReadValid = [readPos, maxReadPos, &DecodingError](uint16 nShouldRead)->bool
	{
		if (readPos + nShouldRead > maxReadPos)
		{
			DecodingError = TRUE;
			return false;
		}
		return true;
	};

	auto IsWriteValid = [NumDecompressedBytes, maxUncompressedBytes, &DecodingError](uint16 nShouldWrite)->bool
	{
		if (NumDecompressedBytes + nShouldWrite > maxUncompressedBytes)
		{
			DecodingError = TRUE;
			return false;
		}
		return true;
	};

	uint16 len = 0;
	while (
				readPos < maxReadPos && 
				NumDecompressedBytes<maxUncompressedBytes
				&&!DecodingError
			)
	{
		auto opcode = *reinterpret_cast<const P2P_PayloadType_e*>(readPos);
		readPos++;

		switch (opcode) 
		{
			case P2P_SamplingRate: //Set sampling rate
			{
				if (!IsReadValid(sizeof(uint16)))
				{
					break;
				}
				readPos += sizeof(uint16);
				break;
			}
			case P2P_Silk: //SILK voice payload
			{
				if (!IsReadValid(sizeof(uint16)))
				{
					break;
				}
				len = *(uint16*)readPos;
				readPos += sizeof(uint16);

				if (!IsReadValid(len))
				{
					break;
				}
				
				NumDecompressedBytes += (m_BackendCodec->Decompress(readPos, len, &pUncompressed[NumDecompressedBytes], (maxUncompressedBytes - NumDecompressedBytes))*BYTES_PER_SAMPLE);
				break;
			}

			case P2P_Raw:
			{
				if (!IsReadValid(sizeof(uint16)))
				{
					break;
				}
				len = *(uint16*)readPos;
				readPos += sizeof(uint16);
				if (!IsReadValid(len))
				{
					break;
				}
				if (!IsWriteValid(len))
				{
					break;
				}
				memmove(&pUncompressed[NumDecompressedBytes], readPos, len);
				NumDecompressedBytes += len;//Mayby, needs checking for parity?
				readPos += len;
				break;
			}

			case P2P_Unknown2Bytes:
			{
				readPos += 2;
			}

			case P2P_UnknownCodec:
			case P2P_Speex: //Somebody managed to attach very old steam to new CS. Lol.
			{
				//Just ignore it.
				if (!IsReadValid(sizeof(uint16)))
				{
					break;
				}
				len = *(uint16*)readPos;
				readPos += sizeof(uint16);
				if (!IsReadValid(len))
				{
					break;
				}
				readPos += len;
			}

			case P2P_Silence:
			{
				if (!IsReadValid(sizeof(uint16)))
				{
					break;
				}
				len = *(uint16*)readPos*BYTES_PER_SAMPLE; //This is number of samples. Not bytes.
				readPos += sizeof(uint16);
				if (!IsWriteValid(len))
				{
					break;
				}
				memset(&pUncompressed[NumDecompressedBytes], 0, len);
				//In fact, we should ignore it in the end of packet.
				NumDecompressedBytes += len;
			}
			default: //Invalid or unknown opcode
				return 0; 
		}
	}

	return NumDecompressedBytes/BYTES_PER_SAMPLE; 
}

//#define P2P_UNCOMPRESSED //Just to prove, that Steam users can hear uncompressed voice data.
int CSteamP2PCodec::StreamEncode(const char *pUncompressedBytes, int nSamples, char *pCompressed, int maxCompressedBytes, bool bFinal) const {
	char* writePos = pCompressed;
	if (maxCompressedBytes < 10) { // no room
		return 0;
	}

	*(writePos++) = P2P_SamplingRate; //set sampling rate
	*(uint16*)writePos = 16000;
	writePos += 2;
#ifndef P2P_UNCOMPRESSED
	*(writePos++) = P2P_Silk; //voice payload

	int compressRes = m_BackendCodec->Compress(pUncompressedBytes, nSamples, writePos + 2, maxCompressedBytes - (1 + 2 + 1 + 2), bFinal);
	if (compressRes == 0) {
		return 0;
	}

	*(uint16*)writePos = compressRes;
	writePos += 2;
	writePos += compressRes;
#else
	if (maxCompressedBytes < nSamples*BYTES_PER_SAMPLE + sizeof(P2P_PayloadType_e)){
		//Buffer too small for uncompressed data.
		return 0;
	}
	*(writePos++) = P2P_Raw; //uncompressed voice payload
	*(uint16*)writePos = nSamples*BYTES_PER_SAMPLE;
	writePos += 2;
	memmove(writePos, pUncompressedBytes, nSamples*BYTES_PER_SAMPLE);
	writePos += nSamples*BYTES_PER_SAMPLE;
#endif

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
