#include "TxtFileReader.h"

void TxtFileReader::skipWhiteSpace()
{
	while ([ch = *m_currentPosition] {
		if (ch <= 32) return ch && ch != '\n';
		else          return ch == '='; }())
		++m_currentPosition;
}

void TxtFileReader::setNextPointer()
{
	m_next = std::find(m_currentPosition, getEndOfFile(), '\n');
}

TxtFileReader::TxtFileReader(const std::filesystem::path& path) : FileReader(path)
{
	skipWhiteSpace();
	setNextPointer();
	if (*m_currentPosition == '\n')
		gotoNextLine();
}

void TxtFileReader::gotoNextLine()
{
	do
	{
		m_currentPosition = m_next;
		if (m_currentPosition == getEndOfFile())
			break;

		m_currentPosition++;
		skipWhiteSpace();

		if (*m_currentPosition == '{')
		{
			m_currentPosition++;
			skipWhiteSpace();
		}
		
		setNextPointer();
	} while (*m_currentPosition == '\n' || strncmp(m_currentPosition, "//", 2) == 0);
}

bool TxtFileReader::doesStringMatch(std::string_view str) const
{
	return strncmp((const char*)m_currentPosition, str.data(), str.size()) == 0;
}

bool TxtFileReader::doesStringMatch_noCase(std::string_view str) const
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		char character = m_currentPosition[i];
		if (65 <= character && character <= 90)
			character += 32;

		if (character != str[i])
			return false;
	}
	return true;
}

void TxtFileReader::skipTrack()
{
	gotoNextLine();
	uint32_t scopeLevel = 1;
	size_t length = strcspn(m_currentPosition, "[}");
	while (m_currentPosition + length != getEndOfFile())
	{
		size_t index = length - 1;
		char point = m_currentPosition[index];
		while (index > 0 && point <= 32 && point != '\n')
			point = m_currentPosition[--index];

		m_currentPosition += length;
		if (point == '\n')
		{
			if (*m_currentPosition == '}')
			{
				if (scopeLevel == 1)
				{
					setNextPointer();
					gotoNextLine();
					return;
				}
				else
					--scopeLevel;
			}
			else
				++scopeLevel;
		}

		length = strcspn(++m_currentPosition, "[}");
	}

	m_next = m_currentPosition = getEndOfFile();
}

std::string_view TxtFileReader::extractText()
{
	auto boundaries = [&]() -> std::pair<const char*, const char*>
	{
		const size_t offset = size_t(1) + (m_next != getEndOfFile());
		if (*m_currentPosition != '\"' || *(m_next - offset) != '\"' || *(m_next - (offset + 1)) == '\\' || m_currentPosition + 1 > m_next - offset)
			return {};

		return { m_currentPosition + 1, m_next - offset };
	}();

	if (!boundaries.first)
		boundaries = { m_currentPosition, m_next - (m_next != getEndOfFile()) };

	while (boundaries.second > boundaries.first && unsigned char(boundaries.second[-1]) <= 32)
		--boundaries.second;

	m_currentPosition = m_next;
	return { boundaries.first, boundaries.second };
}

std::string_view TxtFileReader::parseModifierName()
{
	const char* const start = m_currentPosition;
	while (*m_currentPosition > 32 && *m_currentPosition != '=')
		++m_currentPosition;

	std::string_view name(start, m_currentPosition);
	skipWhiteSpace();
	return name;
}

template <>
bool TxtFileReader::extract(char& value)
{
	if (m_currentPosition >= m_next)
		return false;

	value = *m_currentPosition++;
	skipWhiteSpace();
	return true;
}

template<>
bool TxtFileReader::extract(bool& value)
{
	switch (*m_currentPosition)
	{
	case '0':
		value = false; break;
	case '1':
		value = true; break;
	default:
		value = m_currentPosition + 4 <= m_next &&
			(m_currentPosition[0] == 't' || m_currentPosition[0] == 'T') &&
			(m_currentPosition[1] == 'r' || m_currentPosition[1] == 'R') &&
			(m_currentPosition[2] == 'u' || m_currentPosition[2] == 'U') &&
			(m_currentPosition[3] == 'e' || m_currentPosition[3] == 'E');
	}
	return true;
}
