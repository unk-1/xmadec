#include "xma_stream_decoder.hpp"
#include "wave_format.hpp"
#include "wave_file.hpp"
#include <cstdio>
#include <cassert>
#include <string>

void *decode_full_stream(c_xma_stream_decoder *decoder, size_t *out_size);
void write_wave_file_header(FILE *fp, int channels, int sample_rate, int bits_per_sample, int data_size);

struct s_decoded_stream
{
	c_xma_stream_decoder decoder;
	unsigned char *data = nullptr;
	size_t size = 0;
	s_decoded_stream() = default;
	~s_decoded_stream()
	{
		if (data)
			delete[] data;
	}
} streams[8];

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: xmadec.exe <input.xma> <output.wav>\n");
		return 0;
	}

	c_wave_file wave_file;
	if (!wave_file.open(argv[1]))
	{
		fprintf(stderr, "failed to open input wave file\n");
		return -1;
	}

	char fmt_buff[4096];
	if (!wave_file.get_format_unsafe(fmt_buff, -1))
	{
		fprintf(stderr, "failed read format chunk\n");
		return -1;
	}

	auto *xma_fmt = (XMAWAVEFORMAT*)fmt_buff;

	FILE *outfp;
	if (fopen_s(&outfp, argv[2], "wb+"))
	{
		fprintf(stderr, "failed to open output file\n");
		return -1;
	}

	write_wave_file_header(outfp, 0, 0, 0, 0);

	size_t largest_stream_size = 0;
	for (int i = 0; i < xma_fmt->NumStreams; i++)
	{
		printf("decoding steam #%d...\n", i);
		streams[i].decoder.initialize(&wave_file, xma_fmt, i);
		streams[i].data = (byte*)decode_full_stream(&streams[i].decoder, &streams[i].size);
		if (streams[i].size > largest_stream_size)
			largest_stream_size = streams[i].size;
	}

	int bps = xma_fmt->BitsPerSample;
	size_t in_sample_size = (bps >> 3) * xma_fmt->XmaStreams[0].Channels;
	size_t out_sample_size = in_sample_size * xma_fmt->NumStreams;

	printf("merging samples...\n");

	size_t offset = 0;
	size_t total = 0;
	while (offset < largest_stream_size)
	{
		int channel_no = 0;
		short out_pcm[6];
		for (int i = 0; i < xma_fmt->NumStreams; i++)
		{
			auto stream_fmt = &xma_fmt->XmaStreams[i];
			for (int j = 0; j < stream_fmt->Channels; j++)
			{
				auto in_pcm = (short*)&streams[i].data[offset];
				if ((offset + in_sample_size) <= streams[i].size)
					out_pcm[channel_no] = in_pcm[j];
				else
					out_pcm[channel_no] = 0;

				channel_no++;
			}
		}
		if (fwrite(out_pcm, 1, out_sample_size, outfp) != out_sample_size)
		{
			printf("failed to write samples...\n");
			break;
		}

		total += out_sample_size;
		offset += in_sample_size;
	}
	printf("finalizing...\n");

	fseek(outfp, 0, SEEK_SET);
	write_wave_file_header(outfp, xma_fmt->NumStreams * xma_fmt->XmaStreams[0].Channels, xma_fmt->XmaStreams[0].SampleRate, bps, total);
	fclose(outfp);

	printf("done.\n");
	return 0;
}

void *decode_full_stream(c_xma_stream_decoder *decoder, size_t *out_size)
{
	size_t mem_size = 4096;
	size_t mem_used = 0;
	auto *mem = new unsigned char[mem_size];

	unsigned char buff[4096];
	unsigned int buff_used;
	while (decoder->decode_block(buff, sizeof(buff), &buff_used))
	{
		if (mem_used + buff_used > mem_size)
		{
			mem_size *= 2;
			auto new_mem = new unsigned char[mem_size];
			assert(new_mem);
			if (mem_used > 0)
				memcpy(new_mem, mem, mem_used);
			delete[] mem;
			mem = new_mem;
		}

		memcpy(&mem[mem_used], buff, buff_used);
		mem_used += buff_used;
	}

	*out_size = mem_used;

	return mem;
}

void write_wave_file_header(FILE *fp, int channels, int sample_rate, int bits_per_sample, int data_size)
{
#pragma pack(push, 1)
	struct {
		int riff_chunk_id;
		unsigned int riff_chunk_size;
		int riff_type;
		int fmt_chunk_id;
		unsigned int fmt_chunk_size;
		WAVEFORMATEX wfx;
		int data_chunk_id;
		unsigned int data_chunk_size;
	} header;
#pragma pack(pop)
	static_assert(sizeof(header) == 0x2E, "");

	header.riff_chunk_id = 'FFIR';
	header.riff_chunk_size = data_size + 38;
	header.riff_type = 'EVAW';
	header.fmt_chunk_id = ' tmf';
	header.fmt_chunk_size = 0x12;
	header.wfx.wFormatTag = 1;
	header.wfx.nChannels = channels;
	header.wfx.nSamplesPerSec = sample_rate;
	header.wfx.wBitsPerSample = bits_per_sample;
	header.wfx.nAvgBytesPerSec = ((unsigned int)bits_per_sample >> 3) * sample_rate * (unsigned __int16)channels;
	header.wfx.nBlockAlign = channels * (bits_per_sample >> 3);
	header.wfx.cbSize = 0;
	header.data_chunk_id = 'atad';
	header.data_chunk_size = data_size;
	fwrite(&header, 1, sizeof(header), fp);
}
