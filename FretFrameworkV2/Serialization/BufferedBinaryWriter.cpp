#include "BufferedBinaryWriter.h"

void BufferedBinaryWriter::writeString(std::string_view str)
{
	writeWebType(str.size());
	write(str.data(), str.size());
}

void BufferedBinaryWriter::startBuffer()
{
	m_buffers.emplace_back();
}

void BufferedBinaryWriter::endBuffer()
{
	const std::string buffer = std::move(m_buffers.back());
	m_buffers.pop_back();

	writeWebType(buffer.size());
	write(buffer.data(), buffer.size());
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
