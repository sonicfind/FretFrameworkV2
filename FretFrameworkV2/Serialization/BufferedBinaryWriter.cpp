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

void BufferedBinaryWriter::flushBuffer()
{
	writeWebType((uint32_t)m_buffer.size());
	flushBuffer_NoSize();
}

void BufferedBinaryWriter::flushBuffer_NoSize()
{
	write(m_buffer.data(), m_buffer.size());
	m_buffer.clear();
}

void BufferedBinaryWriter::writeWebType(uint32_t value)
{
	if (value < 254)
		write(value, 1);
	else
	{
		bool is32 = value > UINT16_MAX;
		write<char>(254 + is32);
		write(value, 2 + 2ULL * is32);
	}
}

void BufferedBinaryWriter::appendWebType(uint32_t value)
{
	if (value < 254)
		append(value, 1);
	else
	{
		bool is32 = value > UINT16_MAX;
		append<char>(254 + is32);
		append(value, 2 + 2ULL * is32);
	}
}
