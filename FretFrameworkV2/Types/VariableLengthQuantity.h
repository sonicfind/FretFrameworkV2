#pragma once
#include <stdint.h>
#include <fstream>

namespace VariableLengthQuantity
{
	void write(uint32_t value, std::fstream& outFile);
};
