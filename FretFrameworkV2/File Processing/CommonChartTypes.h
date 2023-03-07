#pragma once
#include <stdint.h>
#include <vector>
#include "TimeSig.h"
#include "SpecialPhrase.h"
enum class ChartEvent : unsigned char
{
	BPM,
	TIME_SIG,
	ANCHOR,
	EVENT,
	SECTION,
	NOTE,
	MULTI_NOTE,
	MODIFIER,
	SPECIAL,
	LYRIC,
	VOCAL,
	VOCAL_PERCUSSION,
	UNKNOWN = 255,
};

enum class TrackType
{
	Info,
	SyncTrack,
	Events,
	Instrument,
	Vocal,
	Diff,
	Animation
};
