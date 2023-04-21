#pragma once
#include <stdint.h>
#include "File Processing/SpecialPhrase.h"

template <class T>
struct Midi_Tracker_Diff
{
	uint32_t notes[T::GetLaneCount()];
	constexpr Midi_Tracker_Diff() { for (uint32_t& note : notes) note = UINT32_MAX; }
};

template <class T>
struct Midi_Tracker_Extensions {};

template <class T>
struct Midi_Tracker
{
	static constexpr int s_defaultLanes[48] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	};

	const unsigned char starPowerNote;

	int laneValues[48];
	uint32_t notes_BRE[5] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	bool doBRE = false;

	std::pair<SpecialPhraseType, uint32_t> solo = { SpecialPhraseType::Solo, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> starPower = { SpecialPhraseType::StarPower, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> tremolo = { SpecialPhraseType::Tremolo, UINT32_MAX };
	std::pair<SpecialPhraseType, uint32_t> trill = { SpecialPhraseType::Trill, UINT32_MAX };

	Midi_Tracker_Diff<T> difficulties[4];
	Midi_Tracker_Extensions<T> ext;

	Midi_Tracker(unsigned char star) : starPowerNote(star)
	{
		memcpy(laneValues, s_defaultLanes, sizeof(s_defaultLanes));
	}
};
