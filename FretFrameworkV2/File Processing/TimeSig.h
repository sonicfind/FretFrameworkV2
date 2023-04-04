#pragma once
#include <stdint.h>
struct TimeSig
{
	unsigned char numerator = 0;
	unsigned char denominator = 255;
	unsigned char metronome = 0;
	unsigned char num32nds = 0;

	TimeSig& combine(TimeSig sig) noexcept
	{
		if (sig.numerator)
			numerator = sig.numerator;

		if (sig.denominator < 255)
			denominator = sig.denominator;

		if (sig.metronome)
			metronome = sig.metronome;

		if (sig.num32nds)
			num32nds = sig.num32nds;
		return *this;
	}
};
