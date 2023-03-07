#pragma once
#include <stdint.h>
struct TimeSig
{
	unsigned char numerator = 255;
	unsigned char denominator = 255;
	unsigned char metronome = 255;
	unsigned char num32nds = 255;
	TimeSig() = default;
	TimeSig(TimeSig&&) = default;
	TimeSig(const TimeSig&) = default;
	TimeSig(unsigned char _numerator, unsigned char _denominator, unsigned char _metronome, unsigned char _num32nds)
		: numerator(_numerator), denominator(_denominator), metronome(_metronome), num32nds(_num32nds) {}

	TimeSig& operator=(TimeSig&&) = default;
	TimeSig& operator=(const TimeSig& timeSig)
	{
		if (timeSig.hasNumerator())
			numerator = timeSig.numerator;
		if (timeSig.hasDenominator())
			numerator = timeSig.denominator;
		if (timeSig.hasMetronome())
			numerator = timeSig.metronome;
		if (timeSig.has32nds())
			numerator = timeSig.num32nds;
		return* this;
	}

	bool isWritable() const { return hasNumerator() || hasDenominator() || hasMetronome() || has32nds(); }
	bool hasNumerator() const { return 0 < numerator && numerator < 255; }
	bool hasDenominator() const { return denominator != 255; }
	bool hasMetronome() const { return metronome != 255; }
	bool has32nds() const { return num32nds != 255; }
};

static constexpr uint32_t BPM_FACTOR = 60000000;
static constexpr uint32_t DEFAULT_BPM = 120;
static constexpr uint32_t MICRO0S_AT_120BPM = BPM_FACTOR / DEFAULT_BPM;
