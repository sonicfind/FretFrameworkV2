#pragma once
#include "TxtFileWriter.h"
#include "Modifiers.h"
class IniFileWriter : public TxtFileWriter
{
public:
	using TxtFileWriter::TxtFileWriter;
	void writeSection(std::string_view str);
	void writeModifiers(const std::vector<Modifiers::Modifier>& list);
};
