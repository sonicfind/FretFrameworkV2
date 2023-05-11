#pragma once
#include <stdint.h>
namespace Midi_Details
{
	struct Hold
	{
		uint64_t start = UINT64_MAX;
		uint64_t end = UINT64_MAX;
		uint64_t getLength() const noexcept { return end - start; }
	};
}
