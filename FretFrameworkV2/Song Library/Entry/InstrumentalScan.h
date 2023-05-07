#pragma once
#include "ScanValues.h"

namespace InstrumentalScan
{
	template <class T>
	[[nodiscard]] bool WasTrackValidated(const ScanValues& values, size_t diff)
	{
		return values.wasTrackValidated(diff);
	}
};
