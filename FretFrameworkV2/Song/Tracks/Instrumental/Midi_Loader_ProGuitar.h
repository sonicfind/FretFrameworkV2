#pragma once
#include "Midi_Loader_Instrument.h"
#include "ProGuitarTrack.h"

template <int numFrets>
struct Midi_Loader_Instrument::Loader_Diff<GuitarNote_Pro<numFrets>>
{
	bool hopo = false;
	uint64_t notes[6] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
	uint64_t slide = UINT64_MAX;
	uint64_t arpeggio = UINT64_MAX;
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::s_noteRange{ 24, 106 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::s_noteRange{ 24, 106 };

template <>
constexpr size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::s_diffValues[96] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

template <>
constexpr size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::s_diffValues[96] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
};

template <>
constexpr unsigned char Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::s_soloValue = 115;

template <>
constexpr unsigned char Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::s_soloValue = 115;

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote_Pro<17>>::Loader_Lanes();

template <>
Midi_Loader_Instrument::Loader_Lanes<GuitarNote_Pro<22>>::Loader_Lanes();

namespace ProGuitar_Midi
{
	constexpr NoteName s_ROOTS[] = { NoteName::E, NoteName::F, NoteName::F_Sharp_Gb, NoteName::G, NoteName::G_Sharp_Ab, NoteName::A, NoteName::A_Sharp_Bb, NoteName::B, NoteName::C, NoteName::C_Sharp_Db, NoteName::D, NoteName::D_Sharp_Eb };

	template <bool NoteOn, int numFrets>
	void ParseColor(DifficultyTrack<GuitarNote_Pro<numFrets>>& track, Midi_Loader_Instrument::Loader_Diff<GuitarNote_Pro<numFrets>>& diffTracker, const uint64_t position, const size_t lane, const unsigned char velocity, unsigned char channel)
	{
		if (lane < 6)
		{
			if constexpr (NoteOn)
			{
				if (channel == 1)
					track.m_arpeggios.get_or_emplace_back(position)[lane].set(velocity - 100);
				else
				{
					auto& guitar = Midi_Loader_Instrument::ConstructNote(track, position);
					if (guitar[lane].m_fret.set(velocity - 100))
					{
						guitar.setHOPO(diffTracker.hopo);
						switch (channel)
						{
						case 2: guitar[lane].setMode(StringMode::Bend); break;
						case 3: guitar[lane].setMode(StringMode::Muted); break;
						case 4: guitar[lane].setMode(StringMode::Tapped); break;
						case 5: guitar[lane].setMode(StringMode::Harmonics); break;
						case 6: guitar[lane].setMode(StringMode::Pinch_Harmonics); break;
						}
						diffTracker.notes[lane] = position;
					}
				}
			}
			else if (channel != 1)
			{
				uint64_t colorPosition = diffTracker.notes[lane];
				if (colorPosition != UINT64_MAX)
				{
					Midi_Loader::GetNode(track.m_notes, colorPosition).object[lane].setLength(position - colorPosition);
					diffTracker.notes[lane] = UINT64_MAX;
				}
			}

		}
		else if (lane == 6)
		{
			diffTracker.hopo = NoteOn;
			if constexpr (NoteOn)
				if (auto note = track.m_notes.try_back(position))
					note->setHOPO(true);
		}
		else if (lane == 7)
		{
			if constexpr (NoteOn)
			{
				track.m_slides.emplace_back(position, channel == 11);
				diffTracker.slide = position;
			}
			else
			{
				track.m_slides.back().setLength(position - diffTracker.slide);
				diffTracker.slide = UINT64_MAX;
			}
		}
		else if (lane == 8)
		{
			if constexpr (NoteOn)
			{
				track.m_arpeggios.get_or_emplace_back(position);
				diffTracker.arpeggio = position;
			}
			else if (diffTracker.arpeggio != UINT64_MAX)
			{
				track.m_arpeggios.back().setLength(position - diffTracker.arpeggio);
				diffTracker.arpeggio = UINT64_MAX;
			}
		}
		else if (lane == 9)
		{
			if constexpr (NoteOn)
			{
				switch (channel)
				{
				case 13: track.m_emphasis.emplace_back(position, EmphasisType::High); break;
				case 14: track.m_emphasis.emplace_back(position, EmphasisType::Middle); break;
				case 15: track.m_emphasis.emplace_back(position, EmphasisType::Low); break;
				}
			}
		}
	}

	template <bool NoteOn, int numFrets>
	void ToggleExtras(InstrumentalTrack<GuitarNote_Pro<numFrets>>& track, Midi_Loader_Instrument::Loader_Ext<GuitarNote_Pro<numFrets>>& ext, const uint64_t position, MidiNote note)
	{
		if constexpr (!NoteOn)
			return;
		
		if (4 <= note.value && note.value <= 15)
		{
			track.m_roots.emplace_back(position, s_ROOTS[note.value - 4]);
			return;
		}

		switch (note.value)
		{
		case 16: track.m_chordPhrases.get_or_emplace_back(position).push_back(ChordPhrase::Slash); break;
		case 17: track.m_chordPhrases.get_or_emplace_back(position).push_back(ChordPhrase::Hide); break;
		case 18: track.m_chordPhrases.get_or_emplace_back(position).push_back(ChordPhrase::Accidental_Switch); break;
		case 107: track.m_chordPhrases.get_or_emplace_back(position).push_back(ChordPhrase::Force_Numbering); break;
		case 108: track.m_handPositions.emplace_back(position, note.velocity - 100); break;
		}
	}
}

template<>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - s_noteRange.first;
	const size_t diff = getDifficulty(noteValue);
	ProGuitar_Midi::ParseColor<NoteOn>(m_track[diff], m_difficulties[diff], m_position, m_lanes.values[noteValue], note.velocity, channel);
}

template<>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - s_noteRange.first;
	const size_t diff = getDifficulty(noteValue);
	ProGuitar_Midi::ParseColor<NoteOn>(m_track[diff], m_difficulties[diff], m_position, m_lanes.values[noteValue], note.velocity, channel);
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::toggleExtraValues(MidiNote note)
{
	ProGuitar_Midi::ToggleExtras<NoteOn>(m_track, m_ext, m_position, note);
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::toggleExtraValues(MidiNote note)
{
	ProGuitar_Midi::ToggleExtras<NoteOn>(m_track, m_ext, m_position, note);
}

