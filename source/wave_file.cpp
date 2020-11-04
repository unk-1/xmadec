#include "wave_file.hpp"
#include <cstdio>
#include <cstdlib>

c_wave_file::~c_wave_file()
{
	close();
}

bool c_wave_file::open(const char * filename)
{
	if (fopen_s((FILE**)&m_fp, filename, "rb"))
		return false;

	return
		find_chunk(m_fp, 'RIFF', &m_riff_chunk) &&
		find_chunk(m_fp, 'fmt ', &m_fmt_chunk) &&
		find_chunk(m_fp, 'data', &m_data_chunk);
}

void c_wave_file::close()
 {
	 if (m_fp)
		 fclose((FILE*)m_fp);
 }

inline size_t c_wave_file::get_format_size()
{
	return m_fmt_chunk.size;
}

bool c_wave_file::get_format_unsafe(char * buff, size_t size)
{
	if (size == -1)
		size = m_fmt_chunk.size;
	return read_chunk_data(m_fp, &m_fmt_chunk, 0, buff, size) == size;
}

bool c_wave_file::read_sample_data(size_t offset, unsigned char * buff, size_t length)
{
	return read_chunk_data(m_fp, &m_data_chunk, offset, buff, length) == length;
}

size_t c_wave_file::get_data_size() const
{
	return m_data_chunk.size;
}

s_riff_chunk * c_wave_file::find_chunk(void *fp, int id, s_riff_chunk * chunk)
{
	id = _byteswap_ulong(id);
	size_t offset = 0;
	while (true)
	{
		fseek((FILE*)fp, offset, SEEK_SET);
		int cb;
		if ((cb = fread(chunk, 1, 8, (FILE*)fp)) != 8)
			return nullptr;

		offset += cb;

		if (chunk->id == id)
		{
			chunk->offset = offset;
			return chunk;
		}

		if (chunk->id == 'FFIR')
			offset += 4;
		else
			offset += chunk->size;
	}

	return nullptr;
}

size_t c_wave_file::read_chunk_data(void * fp, s_riff_chunk * chunk, size_t offset, void * buff, size_t count)
{
	if (offset + count > chunk->size)
		return -1;

	fseek((FILE*)fp, chunk->offset + offset, SEEK_SET);
	return fread(buff, 1, count, (FILE*)fp);
}
