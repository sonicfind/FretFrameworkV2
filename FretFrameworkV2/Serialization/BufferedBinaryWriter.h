#pragma once
#include "BinaryFileWriter.h"

class BufferedBinaryWriter : protected BinaryFileWriter<false>
{
public:
	using BinaryFileWriter::BinaryFileWriter;

	template <typename T>
	void write(const T* const data, size_t size)
	{
		if (!m_buffers.empty())
			m_buffers.back().append((const char*)data, size);
		else
			BinaryFileWriter::write(data, size);
	}

	template <typename T>
	void write(const T& value, size_t size)
	{
		write(&value, size);
	}

	template <typename T>
	void write(const T& value)
	{
		write(value, sizeof(T));
	}

	void writeString(std::string_view str);
	void startBuffer();
	void endBuffer();

protected:
	void writeWebType(uint64_t value);

private:
	std::vector<std::string> m_buffers;
};

