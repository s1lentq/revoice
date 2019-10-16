#include "precompiled.h"

VoiceEncoder_Opus::VoiceEncoder_Opus() : m_bitrate(32000), m_samplerate(8000)
{
	m_nCurFrame = 0;
	m_nLastFrame = 0;
	m_pEncoder = nullptr;
	m_pDecoder = nullptr;
}

VoiceEncoder_Opus::~VoiceEncoder_Opus()
{
	if (m_pEncoder) {
		free(m_pEncoder);
		m_pEncoder = nullptr;
	}

	if (m_pDecoder) {
		free(m_pDecoder);
		m_pDecoder = nullptr;
	}
}

bool VoiceEncoder_Opus::Init(int quality)
{
	m_nCurFrame = 0;
	m_nLastFrame = 0;
	m_PacketLossConcealment = true;

	int encSizeBytes = opus_encoder_get_size(MAX_CHANNELS);
	m_pEncoder = (OpusEncoder *)malloc(encSizeBytes);
	if (opus_encoder_init((OpusEncoder *)m_pEncoder, m_samplerate, MAX_CHANNELS, OPUS_APPLICATION_VOIP) != OPUS_OK) {
		free(m_pEncoder);
		m_pEncoder = nullptr;
		return false;
	}

	opus_encoder_ctl((OpusEncoder *)m_pEncoder, OPUS_SET_BITRATE_REQUEST, m_bitrate);
	opus_encoder_ctl((OpusEncoder *)m_pEncoder, OPUS_SET_SIGNAL_REQUEST, OPUS_SIGNAL_VOICE);
	opus_encoder_ctl((OpusEncoder *)m_pEncoder, OPUS_SET_DTX_REQUEST, 1);

	int decSizeBytes = opus_decoder_get_size(MAX_CHANNELS);
	m_pDecoder = (OpusDecoder *)malloc(decSizeBytes);
	if (opus_decoder_init((OpusDecoder *)m_pDecoder, m_samplerate, MAX_CHANNELS) != OPUS_OK) {
		free(m_pDecoder);
		m_pDecoder = nullptr;
		return false;
	}

	return true;
}

void VoiceEncoder_Opus::Release()
{
	delete this;
}

bool VoiceEncoder_Opus::ResetState()
{
	if (m_pEncoder) {
		opus_encoder_ctl(m_pEncoder, OPUS_RESET_STATE);
	}

	if (m_pDecoder) {
		opus_decoder_ctl(m_pDecoder, OPUS_RESET_STATE);
	}

	m_bufOverflowBytes.Clear();
	return true;
}

int VoiceEncoder_Opus::Compress(const char *pUncompressedIn, int nSamplesIn, char *pCompressed, int maxCompressedBytes, bool bFinal)
{
	if ((nSamplesIn + GetNumQueuedEncodingSamples()) < FRAME_SIZE && !bFinal) {
		m_bufOverflowBytes.Put(pUncompressedIn, nSamplesIn * BYTES_PER_SAMPLE);
		return 0;
	}

	int nSamples = nSamplesIn;
	int nSamplesRemaining = nSamples % FRAME_SIZE;
	char *pUncompressed = (char *)pUncompressedIn;

	if (m_bufOverflowBytes.TellPut() || (nSamplesRemaining && bFinal))
	{
		CUtlBuffer buf;
		buf.Put(m_bufOverflowBytes.Base(), m_bufOverflowBytes.TellPut());
		buf.Put(pUncompressedIn, nSamplesIn * BYTES_PER_SAMPLE);
		m_bufOverflowBytes.Clear();

		nSamples = (buf.TellPut() / 2);
		nSamplesRemaining = nSamples % FRAME_SIZE;

		if (bFinal && nSamplesRemaining)
		{
			for (int i = FRAME_SIZE - nSamplesRemaining; i > 0; i--) {
				buf.PutShort(0);
			}

			nSamples = (buf.TellPut() / 2);
			nSamplesRemaining = nSamples % FRAME_SIZE;
		}

		pUncompressed = (char *)buf.Base();
		Assert(!bFinal || nSamplesRemaining == 0);
	}

	char *psRead = pUncompressed;
	char *pWritePos = pCompressed;
	char *pWritePosMax = pCompressed + maxCompressedBytes;

	int nBlocks = nSamples - nSamplesRemaining;
	while (nBlocks > 0)
	{
		uint16 *pWritePayloadSize = (uint16 *)pWritePos;
		pWritePos += sizeof(uint16); // leave 2 bytes for the frame size (will be written after encoding)

		if (m_PacketLossConcealment)
		{
			*(uint16 *)pWritePos = m_nCurFrame++;
			pWritePos += sizeof(uint16);
		}

		int nBytes = (pWritePosMax - pWritePos > 0x7FFF) ? 0x7FFF : (pWritePosMax - pWritePos);
		int nWriteBytes = opus_encode(m_pEncoder, (const opus_int16 *)psRead, FRAME_SIZE, (unsigned char *)pWritePos, nBytes);

		nBlocks -= FRAME_SIZE;
		psRead += FRAME_SIZE * 2;
		pWritePos += nWriteBytes;

		*pWritePayloadSize = nWriteBytes;
	}

	m_bufOverflowBytes.Clear();

	if (nSamplesRemaining)
	{
		Assert((char *)psRead == pUncompressed + ((nSamples - nSamplesRemaining) * sizeof(uint16)));
		m_bufOverflowBytes.Put(pUncompressed + ((nSamples - nSamplesRemaining) * sizeof(uint16)), 2 * nSamplesRemaining);
	}

	if (bFinal)
	{
		ResetState();

		if (pWritePosMax > pWritePos + 2)
		{
			*(uint16 *)pWritePos = 0xFFFF;
			pWritePos += sizeof(uint16);
		}

		m_nCurFrame = 0;
	}

	return pWritePos - pCompressed;
}

int VoiceEncoder_Opus::Decompress(const char *pCompressed, int compressedBytes, char *pUncompressed, int maxUncompressedBytes)
{
	const char *pReadPos = pCompressed;
	const char *pReadPosMax = &pCompressed[compressedBytes];

	char *pWritePos = pUncompressed;
	char *pWritePosMax = &pUncompressed[maxUncompressedBytes];

	int nPayloadSize;

	while (pReadPos < pReadPosMax)
	{
		nPayloadSize = *(uint16 *)pReadPos;
		pReadPos += sizeof(uint16);

		if (nPayloadSize == 0xFFFF) {
			m_nLastFrame = 0;
			ResetState();
			break;
		}

		if (m_PacketLossConcealment)
		{
			uint16 nCurFrame = *(uint16 *)pReadPos;
			pReadPos += sizeof(uint16);

			if (nCurFrame < m_nLastFrame)
			{
				ResetState();
			}
			else if (nCurFrame != m_nLastFrame)
			{
				int nPacketLoss = nCurFrame - m_nLastFrame;
				if (nPacketLoss > MAX_PACKET_LOSS) {
					nPacketLoss = MAX_PACKET_LOSS;
				}

				for (int i = 0; i < nPacketLoss; i++)
				{
					if (pWritePos + (FRAME_SIZE * 2) >= pWritePosMax)
						break;

					int nBytes = opus_decode(m_pDecoder, 0, 0, (opus_int16 *)pWritePos, FRAME_SIZE, 0);
					if (nBytes <= 0)
					{
						// raw corrupted
						return 0;
					}

					pWritePos += nBytes * 2;
				}
			}

			m_nLastFrame = nCurFrame + 1;
		}

		if ((pReadPos + nPayloadSize) > pReadPosMax) {
			Assert(false);
			break;
		}

		if (pWritePosMax < pWritePos + (FRAME_SIZE * 2)) {
			Assert(false);
			break;
		}

		memset(pWritePos, 0, FRAME_SIZE * 2);

		if (nPayloadSize == 0)
		{
			pWritePos += FRAME_SIZE * 2;
			continue;
		}

		int nBytes = opus_decode(m_pDecoder, (const unsigned char *)pReadPos, nPayloadSize, (opus_int16 *)pWritePos, FRAME_SIZE, 0);
		if (nBytes <= 0)
		{
			// raw corrupted
			return 0;
		}

		pReadPos += nPayloadSize;
		pWritePos += nBytes * 2;
	}

	return (pWritePos - pUncompressed) / BYTES_PER_SAMPLE;
}
