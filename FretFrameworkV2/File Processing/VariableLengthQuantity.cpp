#include "VariableLengthQuantity.h"
#include <string>

void VariableLengthQuantity::write(uint32_t value, std::fstream& outFile)
{
	if (value & (15 << 28))
		throw std::runtime_error("Integer value cannot exceed 134217728 (value: " + std::to_string(value) + ")");

	static char writeBuffer[4];
	char* bufferStart = writeBuffer + 3;
	size_t bufferSize = 1;

	*bufferStart = value & 127;
	for (value >>= 7; value > 0; value >>= 7)
	{
		*--bufferStart = (value & 127) | 128;
		++bufferSize;
	}
	outFile.write(bufferStart, bufferSize);
}
