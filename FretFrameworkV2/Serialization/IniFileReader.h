#pragma once
#include "TxtFileReader.h"
class IniFileReader : public TxtFileReader
{
public:
	using TxtFileReader::TxtFileReader;
	[[nodiscard]] bool isStartOfSection() const;
	[[nodiscard]] bool validateSection(std::string_view str);
	void skipSection();


public:
	std::vector<Modifiers::Modifier> extractModifiers(const ModifierOutline& list);

private:
	[[nodiscard]] bool isStillCurrentSection();
};
