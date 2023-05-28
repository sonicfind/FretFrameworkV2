#pragma once
#include "BinaryFileReader.h"

class BufferedBinaryReader : public BinaryFileReader
{
public:
	using BinaryFileReader::BinaryFileReader;
	[[nodiscard]] std::string_view extractString();
	void enterSection();

protected:
	[[nodiscard]] std::string_view extractString(uint64_t length);

	bool extractWebType(uint64_t& value);
	[[nodiscard]] uint64_t extractWebType();
};

