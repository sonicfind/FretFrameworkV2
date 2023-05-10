#include "BufferedBinaryReader.h"

std::string_view BufferedBinaryReader::extractString()
{
	const uint64_t length = extractWebType();
	return extractString(length);
}

void BufferedBinaryReader::setNextSectionBounds()
{
	const uint32_t length = extractWebType<false>();
	m_next = m_currentPosition + length;

	if (m_next > m_file.end())
		throw std::runtime_error("Invalid length for section; Extends past EOF");
}

void BufferedBinaryReader::gotoEndOfBuffer()
{
	m_currentPosition = m_next;
}

bool BufferedBinaryReader::move(size_t amount)
{
	if (m_currentPosition + amount > m_next)
		return false;

	return FileReader::move(amount);
}

std::string_view BufferedBinaryReader::extractString(size_t length)
{
	std::string_view str(m_currentPosition, length);
	if (!move(length))
		throw std::runtime_error("length of text invalid");
	return str;
}
