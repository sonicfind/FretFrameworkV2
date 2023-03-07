#pragma once
#include <stdint.h>
#include <fstream>

namespace WebType
{
	uint32_t extract(const char*& dataPtr);
	void copyToBuffer(const uint32_t value, char*& buffer);
	void writeToFile(const uint32_t value, std::fstream& outFile);
}
