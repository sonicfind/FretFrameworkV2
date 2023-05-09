#pragma once
#include <stdint.h>
#include <vector>
#include "Song/TimeSig.h"
#include "Notes/SpecialPhrase.h"
#include "Notes/Pitch.h"

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
	NOTE_PRO,
	MUTLI_NOTE_PRO,
	ROOT,
	LEFT_HAND,
	PITCH,
	RANGE_SHIFT,
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
