#pragma once
#include "Midi_Loader_Instrument.h"
#include "ProGuitarTrack.h"

template <int numFrets>
struct Midi_Loader_Instrument::Loader_Lanes<GuitarNote_Pro<numFrets>> {};

template <int numFrets>
struct Midi_Loader_Instrument::Loader_Diff<GuitarNote_Pro<numFrets>>
{
	static constexpr std::pair<unsigned char, unsigned char> NOTERANGE{ 24, 106 };
	bool hopo = false;
	uint64_t notes[6] = { UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
	uint64_t arpeggio = UINT64_MAX;
	ProSlide slide = ProSlide::None;
	EmphasisType emphasis = EmphasisType::None;
};

template <>
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::getDifficulty(size_t noteValue) const noexcept;

template <>
size_t Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::getDifficulty(size_t noteValue) const noexcept;

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::Loader(InstrumentalTrack<GuitarNote_Pro<17>>& track, unsigned char multiplierNote);

template <>
Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::Loader(InstrumentalTrack<GuitarNote_Pro<22>>& track, unsigned char multiplierNote);

namespace Midi_Loader_ProGuitar
{
	constexpr NoteName s_ROOTS[] = { NoteName::E, NoteName::F, NoteName::F_Sharp_Gb, NoteName::G, NoteName::G_Sharp_Ab, NoteName::A, NoteName::A_Sharp_Bb, NoteName::B, NoteName::C, NoteName::C_Sharp_Db, NoteName::D, NoteName::D_Sharp_Eb };
	constexpr size_t LANES[96] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	};

	size_t DIFF(size_t noteValue);

	template <bool NoteOn, int numFrets>
	void ParseColor(DifficultyTrack<GuitarNote_Pro<numFrets>>& track, Midi_Loader_Instrument::Loader_Diff<GuitarNote_Pro<numFrets>>& diffTracker, const uint64_t position, const size_t noteValue, const unsigned char velocity, unsigned char channel)
	{
		size_t lane = LANES[noteValue];
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
						guitar.setHOPO(diffTracker.hopo).setSlide(diffTracker.slide).setEmphasis(diffTracker.emphasis);
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
				diffTracker.slide = channel == 11 ? ProSlide::Reversed : ProSlide::Normal;
				if (auto note = track.m_notes.try_back(position))
					note->setSlide(diffTracker.slide);
			}
			else
				diffTracker.slide = ProSlide::None;
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
				case 13: diffTracker.emphasis = EmphasisType::High; break;
				case 14: diffTracker.emphasis = EmphasisType::Middle; break;
				case 15: diffTracker.emphasis = EmphasisType::Low; break;
				default: return;
				}

				if (auto note = track.m_notes.try_back(position))
					note->setEmphasis(diffTracker.emphasis);
			}
			else
				diffTracker.emphasis = EmphasisType::None;
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
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	Midi_Loader_ProGuitar::ParseColor<NoteOn>(m_track[diff], m_difficulties[diff], m_position, noteValue, note.velocity, channel);
}

template<>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::parseLaneColor(MidiNote note, unsigned char channel)
{
	const size_t noteValue = note.value - m_difficulties->NOTERANGE.first;
	const size_t diff = getDifficulty(noteValue);
	Midi_Loader_ProGuitar::ParseColor<NoteOn>(m_track[diff], m_difficulties[diff], m_position, noteValue, note.velocity, channel);
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<17>>::toggleExtraValues(MidiNote note)
{
	Midi_Loader_ProGuitar::ToggleExtras<NoteOn>(m_track, m_ext, m_position, note);
}
template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<GuitarNote_Pro<22>>::toggleExtraValues(MidiNote note)
{
	Midi_Loader_ProGuitar::ToggleExtras<NoteOn>(m_track, m_ext, m_position, note);
}
