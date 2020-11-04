#pragma once

class c_wave_file;
struct XMAWAVEFORMAT;

class c_xma_stream_decoder
{
public:
	~c_xma_stream_decoder();
	bool initialize(c_wave_file *wave_file, XMAWAVEFORMAT *xma_fmt, int stream_index);
	bool decode_block(unsigned char *buff, unsigned int buff_size, unsigned int *buff_used);
	void dispose();

private:
	bool m_initialized;
	c_wave_file *m_wave_file = nullptr;
	void *m_paudec = nullptr;
	int m_state = 0;
	unsigned char m_block[2048];
	unsigned int m_block_size;
	unsigned int m_samples_ready;
	size_t m_stream_offset;
	size_t m_data_offset;
	size_t m_xma_block_offset;
	size_t m_frame_skip;
};
