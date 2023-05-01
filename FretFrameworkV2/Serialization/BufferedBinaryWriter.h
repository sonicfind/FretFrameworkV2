#pragma once
#include "BinaryFileWriter.h"

class BufferedBinaryWriter : public BinaryFileWriter<false>
{
public:
	using BinaryFileWriter::BinaryFileWriter;

	template <typename T>
	void append(const T& value)
	{
		append(value, sizeof(T));
	}

	void writeString(std::string_view str);
	void appendString(std::string_view str);
	void writeBuffer();

protected:
	template <typename T>
	void append(const T* const data, size_t size)
	{
		m_buffer.append((const char*)data, size);
	}

	template <typename T>
	void append(const T& value, size_t size)
	{
		append(&value, size);
	}

	void writeWebType(uint32_t value);
	void appendWebType(uint32_t value);

private:
	std::string m_buffer;
};

