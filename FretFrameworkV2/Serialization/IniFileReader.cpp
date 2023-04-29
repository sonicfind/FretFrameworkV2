#include "IniFileReader.h"

bool IniFileReader::isStartOfSection() const
{
	return *m_currentPosition == '[';
}

bool IniFileReader::validateSection(std::string_view str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		char character = m_currentPosition[i];
		if (65 <= character && character <= 90)
			character += 32;

		if (character != str[i])
			return false;
	}

	gotoNextLine();
	return true;
}

void IniFileReader::skipSection()
{
	gotoNextLine();
	const char* curr = m_currentPosition;
	while (const char* next = strchr(curr, '['))
	{
		const char* point = next - 1;
		while (point > curr && *point <= 32 && *point != '\n')
			--point;

		if (*point == '\n')
		{
			m_currentPosition = next;
			setNextPointer();
			return;
		}

		curr = next + 1;
	}

	m_next = m_currentPosition = getEndOfFile();
}

bool IniFileReader::isStillCurrentSection()
{
	return *m_currentPosition != 0 && *m_currentPosition != '[';
}

std::vector<Modifiers::Modifier> IniFileReader::extractModifiers(const ModifierOutline& list)
{
	std::vector<Modifiers::Modifier> modifiers;
	while (isStillCurrentSection())
	{
		if (auto node = findNode(extractModifierName(), list))
			modifiers.push_back(createModifier(*node));
		gotoNextLine();
	}
	return modifiers;
}
