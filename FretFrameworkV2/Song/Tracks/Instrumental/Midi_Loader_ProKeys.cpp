#include "Midi_Loader_ProKeys.h"

template <>
Midi_Loader_Instrument::Loader<Keys_Pro>::Loader(InstrumentalTrack<Keys_Pro>& track, unsigned char multiplierNote)
	: m_track(track), m_phrases({
	{ { 115 }, { SpecialPhraseType::Solo } },
	{ { multiplierNote }, { SpecialPhraseType::StarPower } },
	{ { 120 }, { SpecialPhraseType::StarPowerActivation_or_BRE } },
	{ { 126 }, { SpecialPhraseType::Glissando } },
	{ { 127 }, { SpecialPhraseType::Trill } },
}) {}
