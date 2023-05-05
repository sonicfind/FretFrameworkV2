#pragma once
#include <stdint.h>
namespace Midi_Details
{
	struct Hold
	{
		uint32_t start = UINT32_MAX;
		uint32_t end = UINT32_MAX;
	};
}
