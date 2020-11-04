#pragma once

struct s_riff_chunk
{
	int id;
	size_t size;
	size_t offset;
};

class c_wave_file
{
public:
	~c_wave_file();

	size_t get_format_size();
	size_t get_data_size() const;

	bool open(const char *filename);
	void close();
	bool get_format_unsafe(char *buff, size_t size);
	bool read_sample_data(size_t offset, unsigned char *buff, size_t length);
	
	s_riff_chunk *find_chunk(void *fp, int id, s_riff_chunk *chunk);
	size_t read_chunk_data(void *fp, s_riff_chunk *chunk, size_t offset, void *buff, size_t count);

private:
	void *m_fp;
	s_riff_chunk m_riff_chunk;
	s_riff_chunk m_data_chunk;
	s_riff_chunk m_fmt_chunk;
};
