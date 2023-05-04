#pragma once
#include <stdint.h>
struct TimeSig
{
	unsigned char numerator = 0;
	unsigned char denominator = 255;
	unsigned char metronome = 0;
	unsigned char num32nds = 0;
};
