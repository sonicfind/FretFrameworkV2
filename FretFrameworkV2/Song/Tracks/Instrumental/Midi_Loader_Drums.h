#pragma once
#include "Midi_Loader_Instrument.h"
#include "Notes/DrumNote_Legacy.h"

template <class DrumType, size_t numPads>
struct Midi_Loader_Instrument::Loader_Diff<DrumNote<DrumType, numPads>>
{
	uint64_t notes[numPads + 1]{ UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX, UINT64_MAX };
	Loader_Diff() { if constexpr (numPads == 5) notes[5] = UINT64_MAX; }
};

template <class DrumType, size_t numPads>
struct Midi_Loader_Instrument::Loader_Ext<DrumNote<DrumType, numPads>>
{
	using PRO_DRUMS = std::is_same<DrumType, DrumPad_Pro>;
	bool enableDynamics = false;
	bool toms[3] = { !PRO_DRUMS::value, !PRO_DRUMS::value, !PRO_DRUMS::value };
	bool flams[4]{};
};

template <>
struct Midi_Loader_Instrument::Loader_Diff<DrumNote_Legacy> : public Midi_Loader_Instrument::Loader_Diff<DrumNote<DrumPad_Pro, 5>> {};

template <>
struct Midi_Loader_Instrument::Loader_Ext<DrumNote_Legacy> : public Midi_Loader_Instrument::Loader_Ext<DrumNote<DrumPad_Pro, 5>> {};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::s_noteRange{ 60, 102 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader_Instrument::Loader<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template<>
template <bool NoteOn>
bool Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::processSpecialNote(MidiNote note)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = m_position;
		constructNote(m_track[3]).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0);
	return true;
}

template<>
template <bool NoteOn>
bool Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::processSpecialNote(MidiNote note)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = m_position;
		constructNote(m_track[3]).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0);
	return true;
}

template<>
template <bool NoteOn>
bool Midi_Loader_Instrument::Loader<DrumNote_Legacy>::processSpecialNote(MidiNote note)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = m_position;
		constructNote(m_track[3]).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0);
	return true;
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::modNote(DrumNote<DrumPad_Pro, 4>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::modNote(DrumNote<DrumPad, 5>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader_Instrument::Loader<DrumNote_Legacy>::modNote(DrumNote_Legacy& note, size_t diff, size_t lane, unsigned char velocity);

template <bool NoteOn, class DrumType, size_t numPads, class DrumNoteClass>
void ToggleExtras(Midi_Loader_Instrument::Loader_Ext<DrumNote<DrumType, numPads>>& ext, InstrumentalTrack<DrumNoteClass>& track, uint64_t position, unsigned char midiValue)
{
	if (midiValue == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			ext.flams[i] = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = track[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
	else if constexpr (std::is_same<DrumType, DrumPad_Pro>::value)
	{
		if (110 <= midiValue && midiValue <= 112)
		{
			ext.toms[midiValue - 110] = NoteOn;
			if constexpr(std::is_same<DrumNoteClass, DrumNote_Legacy>::value)
				DrumNote_Legacy::Signal4Pro();
		}
	}
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::toggleExtraValues(MidiNote note)
{
	ToggleExtras<NoteOn>(m_ext, m_track, m_position, note.value);
}

template <>
template <bool NoteOn>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::toggleExtraValues(MidiNote note)
{
	ToggleExtras<NoteOn>(m_ext, m_track, m_position, note.value);
}

template <>
template <bool NoteOn>
void  Midi_Loader_Instrument::Loader<DrumNote_Legacy>::toggleExtraValues(MidiNote note)
{
	ToggleExtras<NoteOn>(m_ext, m_track, m_position, note.value);
}

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad_Pro, 4>>::parseText(std::string_view str);

template <>
void Midi_Loader_Instrument::Loader<DrumNote<DrumPad, 5>>::parseText(std::string_view str);

template <>
void Midi_Loader_Instrument::Loader<DrumNote_Legacy>::parseText(std::string_view str);
