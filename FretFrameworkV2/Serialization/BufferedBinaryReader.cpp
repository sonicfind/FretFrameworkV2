#include "BufferedBinaryReader.h"

std::string_view BufferedBinaryReader::extractString()
{
	const uint64_t length = extractWebType();
	return extractString(length);
}

void BufferedBinaryReader::enterSection()
{
	const uint64_t length = extractWebType();
	const char* const end = m_currentPosition + length;
	if (end > m_ends.back())
		throw std::runtime_error("Invalid length for section");
	m_ends.push_back(end);
}

void BufferedBinaryReader::exitSection()
{
	m_currentPosition = m_ends.back();
	m_ends.pop_back();
}

std::string_view BufferedBinaryReader::extractString(size_t length)
{
	std::string_view str(m_currentPosition, length);
	if (!move(length))
		throw std::runtime_error("length of text invalid");
	return str;
}

bool BufferedBinaryReader::extractWebType(uint64_t& value)
{
	value = 0;
	if (!extract<uint64_t>(value, 1))
		return false;
	return value < 253 || extract<uint64_t>(value, 2 + 2 * ((value > 253) + 2 * (value > 254)));
}

[[nodiscard]] uint64_t BufferedBinaryReader::extractWebType()
{
	uint64_t value;
	if (!extractWebType(value))
		throw std::runtime_error("can not parse this data");
	return value;
}
