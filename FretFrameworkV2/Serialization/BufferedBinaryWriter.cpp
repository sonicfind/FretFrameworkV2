#include "BufferedBinaryWriter.h"

void BufferedBinaryWriter::writeString(std::string_view str)
{
	writeWebType((uint32_t)str.size());
	write(str.data(), str.size());
}

void BufferedBinaryWriter::appendString(std::string_view str)
{
	appendWebType((uint32_t)str.size());
	append(str.data(), str.size());
}

void BufferedBinaryWriter::writeBuffer()
{
	writeWebType((uint32_t)m_buffer.size());
	write<char>(m_buffer.data(), m_buffer.size());
	m_buffer.clear();
}

void BufferedBinaryWriter::writeWebType(uint64_t value)
{
	if (value < 253)
		write(value, 1);
	else
	{
		bool exceeds16 = value > UINT16_MAX;
		bool exceeds32 = value > UINT32_MAX;
		write<char>(253 + exceeds16 + exceeds32);
		write(value, 2 + 2ULL * (exceeds16 + 2 * exceeds32));
	}
}

void BufferedBinaryWriter::appendWebType(uint64_t value)
{
	if (value < 253)
		append(value, 1);
	else
	{
		bool exceeds16 = value > UINT16_MAX;
		bool exceeds32 = value > UINT32_MAX;
		append<char>(253 + exceeds16 + exceeds32);
		append(value, 2 + 2ULL * (exceeds16 + 2 * exceeds32));
	}
}
