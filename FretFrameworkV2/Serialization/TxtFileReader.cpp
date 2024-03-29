#include "TxtFileReader.h"

void TxtFileReader::skipWhiteSpace()
{
	while ([ch = (unsigned char)*m_currentPosition] {
		if (ch <= 32) return ch && ch != '\n';
		else          return ch == '='; }())
		++m_currentPosition;
}

void TxtFileReader::setNextPointer()
{
	m_next = std::find(m_currentPosition, m_file.end(), '\n');
}

void TxtFileReader::startRead()
{
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0 };
	if (strncmp(m_currentPosition, BOM, 3) == 0)
		m_currentPosition += 3;

	skipWhiteSpace();
	setNextPointer();
	if (*m_currentPosition == '\n')
		gotoNextLine();
}

TxtFileReader::TxtFileReader(const std::filesystem::path& path) : FileReader(path)
{
	startRead();
}

TxtFileReader::TxtFileReader(const LoadedFile& file) : FileReader(file)
{
	startRead();
}

void TxtFileReader::gotoNextLine()
{
	do
	{
		m_currentPosition = m_next;
		if (m_currentPosition == m_file.end())
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

std::string_view TxtFileReader::extractText(bool checkForQuotes)
{
	std::pair<const char*, const char*> boundaries = [&]() -> std::pair<const char*, const char*>
	{
		const char* const endOfLine = m_next - (m_next != m_file.end());
		if (checkForQuotes && *m_currentPosition == '\"')
		{
			const char* end = endOfLine - 1;
			while (m_currentPosition + 1 < end && (unsigned char)*end <= 32)
				--end;

			if (m_currentPosition < end && *end == '\"' && end[-1] != '\\')
				return { m_currentPosition + 1, end };
		}
		return { m_currentPosition, endOfLine };
	}();

	if (boundaries.second < boundaries.first)
		return { boundaries.first, boundaries.first };

	while (boundaries.second > boundaries.first && unsigned char(boundaries.second[-1]) <= 32)
		--boundaries.second;

	m_currentPosition = m_next;
	return { boundaries.first, boundaries.second };
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

std::string_view TxtFileReader::extractModifierName()
{
	const char* const start = m_currentPosition;
	while (*m_currentPosition > 32 && *m_currentPosition != '=')
		++m_currentPosition;

	std::string_view name(start, m_currentPosition);
	skipWhiteSpace();
	return name;
}

std::optional<TxtFileReader::ModifierNode> TxtFileReader::findNode(std::string_view name, const std::vector<std::pair<std::string_view, ModifierNode>>& list)
{
	const auto pairIter = std::lower_bound(std::begin(list), std::end(list), name,
		[](const std::pair<std::string_view, ModifierNode>& pair, const std::string_view str)
		{
			return pair.first < str;
		});

	if (pairIter == std::end(list) || name != pairIter->first)
		return {};

	return pairIter->second;
}

Modifiers::Modifier TxtFileReader::createModifier(ModifierNode node)
{
	try
	{
		switch (node.type)
		{
		case ModifierNode::STRING:              return { node.name, UnicodeString(extractText(false)) };
		case ModifierNode::STRING_CHART:        return { node.name, UnicodeString(extractText()) };
		case ModifierNode::STRING_NOCASE:       return { node.name, UnicodeString::strToU32(extractText(false)) };
		case ModifierNode::STRING_CHART_NOCASE: return { node.name, UnicodeString::strToU32(extractText()) };
		case ModifierNode::UINT64:              return { node.name, extract<uint64_t>() };
		case ModifierNode::INT64:               return { node.name, extract<int64_t>() };
		case ModifierNode::UINT32:              return { node.name, extract<uint32_t>() };
		case ModifierNode::INT32:               return { node.name, extract<int32_t>() };
		case ModifierNode::UINT16:              return { node.name, extract<uint16_t>() };
		case ModifierNode::INT16:               return { node.name, extract<int16_t>() };
		case ModifierNode::BOOL:                return { node.name, extract<bool>() };
		case ModifierNode::FLOAT:               return { node.name, extract<float>() };
		case ModifierNode::FLOATARRAY:
		{
			const float flt1 = extract<float>();
			const float flt2 = extract<float>();
			return { node.name, flt1, flt2 };
		}
		}
	}
	catch (...)
	{
		switch (node.type)
		{
		case ModifierNode::UINT64:     return { node.name, uint64_t(0) };
		case ModifierNode::INT64:      return { node.name, int64_t(0) };
		case ModifierNode::UINT32:     return { node.name, uint32_t(0) };
		case ModifierNode::INT32:      return { node.name, int32_t(0) };
		case ModifierNode::UINT16:     return { node.name, uint16_t(0) };
		case ModifierNode::BOOL:       return { node.name, false };
		case ModifierNode::FLOAT:      return { node.name, .0f };
		case ModifierNode::FLOATARRAY: return { node.name, 0, 0 };
		}
	}
	throw std::runtime_error("How in the fu-");
}
