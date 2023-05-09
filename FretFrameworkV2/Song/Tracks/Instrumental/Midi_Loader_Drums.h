#pragma once
#include "Midi_Loader_Instrument.h"
#include "Notes/DrumNote_Legacy.h"

template <size_t numPads, bool PRO_DRUMS>
struct Midi_Loader_Diff<DrumNote<numPads, PRO_DRUMS>>
{
	bool flam = false;
	uint32_t notes[numPads + 1]{ UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	Midi_Loader_Diff() { if constexpr (numPads == 5) notes[5] = UINT32_MAX; }
};

template <size_t numPads, bool PRO_DRUMS>
struct Midi_Loader_Extensions<DrumNote<numPads, PRO_DRUMS>>
{
	bool enableDynamics = false;
	bool toms[3] = { !PRO_DRUMS, !PRO_DRUMS, !PRO_DRUMS };
};

template <>
struct Midi_Loader_Diff<DrumNote_Legacy> : public Midi_Loader_Diff<DrumNote<5, true>> {};

template <>
struct Midi_Loader_Extensions<DrumNote_Legacy> : public Midi_Loader_Extensions<DrumNote<5, true>>
{
	DrumType_Enum type;
};

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader<DrumNote<5, false>>::s_noteRange{ 60, 102 };

template <>
constexpr std::pair<unsigned char, unsigned char> Midi_Loader<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template<>
template <bool NoteOn>
bool Midi_Loader<DrumNote<4, true>>::processSpecialNote(MidiNote note, uint32_t position)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = position;
		constructNote(m_track[3], position).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0, position);
	return true;
}

template<>
template <bool NoteOn>
bool Midi_Loader<DrumNote<5, false>>::processSpecialNote(MidiNote note, uint32_t position)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = position;
		constructNote(m_track[3], position).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0, position);
	return true;
}

template<>
template <bool NoteOn>
bool Midi_Loader<DrumNote_Legacy>::processSpecialNote(MidiNote note, uint32_t position)
{
	if (note.value != 95)
		return false;

	if constexpr (NoteOn)
	{
		m_difficulties[3].notes[0] = position;
		constructNote(m_track[3], position).modify('+');
	}
	else
		addColor(m_track[3].m_notes, 3, 0, position);
	return true;
}

template <>
void Midi_Loader<DrumNote<4, true>>::modNote(DrumNote<4, true>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader<DrumNote<5, false>>::modNote(DrumNote<5, false>& note, size_t diff, size_t lane, unsigned char velocity);

template <>
void Midi_Loader<DrumNote_Legacy>::modNote(DrumNote_Legacy& note, size_t diff, size_t lane, unsigned char velocity);

template <>
template <bool NoteOn>
void Midi_Loader<DrumNote<4, true>>::toggleExtraValues(MidiNote note, uint32_t position)
{
	if (note.value == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			m_difficulties[i].flam = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = m_track[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
	else if (110 <= note.value && note.value <= 112)
		m_ext.toms[note.value - 110] = NoteOn;
}

template <>
template <bool NoteOn>
void Midi_Loader<DrumNote<5, false>>::toggleExtraValues(MidiNote note, uint32_t position)
{
	if (note.value == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			m_difficulties[i].flam = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = m_track[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
}

template <>
template <bool NoteOn>
void  Midi_Loader<DrumNote_Legacy>::toggleExtraValues(MidiNote note, uint32_t position)
{
	if (note.value == 109)
	{
		for (size_t i = 0; i < 4; ++i)
		{
			m_difficulties[i].flam = NoteOn;
			if constexpr (NoteOn)
				if (auto drum = m_track[i].m_notes.try_back(position))
					drum->setFlam(true);
		}
	}
	else if (110 <= note.value && note.value <= 112 && m_ext.type != DrumType_Enum::FIVELANE)
	{
		m_ext.toms[note.value - 110] = NoteOn;
		m_ext.type = DrumType_Enum::FOURLANE_PRO;
	}
}

template <>
void Midi_Loader<DrumNote<4, true>>::parseText(std::string_view str, uint32_t position);

template <>
void Midi_Loader<DrumNote<5, false>>::parseText(std::string_view str, uint32_t position);

template <>
void Midi_Loader<DrumNote_Legacy>::parseText(std::string_view str, uint32_t position);
