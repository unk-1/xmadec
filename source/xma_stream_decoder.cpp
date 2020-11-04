#include "xma_stream_decoder.hpp"
#include "msaudio.hpp"
#include "wave_format.hpp"
#include "wave_file.hpp"
#include <cassert>
#include <cstdio>

static int map_sample_rate(int sample_rate)
{
	if (sample_rate > 24000)
	{
		if (sample_rate > 32000)
		{
			if (sample_rate > 44100)
				sample_rate = 48000;
			else
				sample_rate = 44100;
		}
		else
			sample_rate = 32000;
	}
	else
		sample_rate = 24000;

	return sample_rate;
}


c_xma_stream_decoder::~c_xma_stream_decoder()
{
	dispose();
}

bool c_xma_stream_decoder::initialize(c_wave_file * wave_file, XMAWAVEFORMAT * xma_fmt, int stream_index)
{
	m_wave_file = wave_file;
	m_paudec = audecNew(nullptr, 0);
	if (!m_paudec)
		return false;

	auto stream = &xma_fmt->XmaStreams[stream_index];

	WMAFormat wma_fmt{};
	PCMFormat pcm_fmt{};

	if (stream->Channels != 1 && stream->Channels != 2)
		return false;

	unsigned short encoder_flags = 0x10D6;

	int sample_rate = map_sample_rate(stream->SampleRate);
	if (msaudioGetSamplePerFrame(sample_rate, 8 * stream->PsuedoBytesPerSec, 3, encoder_flags) != 512)
		return false;

	wma_fmt.wFormatTag = 0x162;
	wma_fmt.nChannels = stream->Channels;
	wma_fmt.nSamplesPerSec = sample_rate;
	wma_fmt.nAvgBytesPerSec = stream->PsuedoBytesPerSec;
	wma_fmt.nBlockAlign = 2048;
	wma_fmt.nValidBitsPerSample = 16;
	wma_fmt.nChannelMask = 0;
	wma_fmt.wEncodeOpt = encoder_flags;
	pcm_fmt.nSamplesPerSec = sample_rate;
	pcm_fmt.nChannels = stream->Channels;
	pcm_fmt.nChannelMask = 0;
	pcm_fmt.nValidBitsPerSample = 16;
	pcm_fmt.cbPCMContainerSize = 2;
	pcm_fmt.pcmData = PCMDataPCM;

	WMAPlayerInfo player_info{};
	if (audecInit(m_paudec, &wma_fmt, &pcm_fmt, &player_info, (audecState*)&m_state, nullptr) < 0)
		return false;

	m_stream_offset = stream_index << 11;
	m_frame_skip = stream->Channels << 7;
	m_xma_block_offset = 0;
	m_data_offset = 0;

	m_initialized = true;
	return true;
}

bool c_xma_stream_decoder::decode_block(unsigned char * buff, unsigned int buff_size, unsigned int * buff_used)
{
	assert(m_initialized);

	bool eof = false;
	for (;;)
	{
		if (m_state == 1)
		{
			auto size = m_wave_file->get_data_size();
			auto block_size = m_wave_file->get_data_size() - m_data_offset;
			if (block_size == 0)
				return false;

			if (block_size > INT_MAX)
			{
				fprintf(stderr, "unexpected eof\n");
				return false;
			}

			if (block_size > sizeof(m_block))
				block_size = sizeof(m_block);

			if (!m_wave_file->read_sample_data(m_data_offset + m_stream_offset, m_block, block_size))
			{
				fprintf(stderr, "failed to read samples\n");
				return false;
			}

			if (!(m_xma_block_offset & 0x7FF))
				*(DWORD *)m_block = *(DWORD *)m_block & 0xFFFEFF08 | ((m_xma_block_offset & 0x7800 | 0x400) >> 7);

			m_data_offset += (m_block[3] + 1) << 11;
			m_xma_block_offset += 2048;
			m_block_size = block_size;

			if (audecInput(m_paudec, m_block, block_size, 1, 0, 0, 0, (audecState*)&m_state, nullptr) < 0)
			{
				fprintf(stderr, "audecInput() failed.\n");
				return false;
			}
		}
		else if (m_state == 2)
		{
			if (audecDecode(m_paudec, &m_samples_ready, (audecState*)&m_state, nullptr) < 0)
			{
				fprintf(stderr, "audecDecode() failed.\n");
				return false;
			}

		}
		else if (m_state == 3)
		{
			unsigned int samples_returned;
			if (audecGetPCM(m_paudec, m_samples_ready, &samples_returned,
				buff, buff_size, buff_used, nullptr, (audecState*)&m_state, nullptr, nullptr) < 0)
			{
				fprintf(stderr, "audecGetPCM() failed.\n");
				return false;
			}

			m_samples_ready -= samples_returned;

			if (m_frame_skip > 0)
			{
				if (m_frame_skip > * buff_used)
					continue;

				if (m_frame_skip <= *buff_used)
				{
					*buff_used -= m_frame_skip;
					memcpy(buff, &buff[m_frame_skip], *buff_used);
					m_frame_skip = 0;
				}
			}

			break;
		}
	}

	return true;
}

inline void c_xma_stream_decoder::dispose()
{
	if (m_paudec)
		audecDelete(m_paudec);
}
