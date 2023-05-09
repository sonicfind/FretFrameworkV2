#include "Midi_Scan_Drums.h"

template <>
bool Midi_Scanner<DrumNote<DrumPad_Pro, 4>>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31;
}

template <>
bool Midi_Scanner<DrumNote<DrumPad, 5>>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31;
}

template <>
bool Midi_Scanner<DrumNote_Legacy>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31 && m_ext.type != DrumType_Enum::LEGACY;
}

template <>
bool Midi_Scanner<DrumNote_Legacy>::processExtraValues(MidiNote note)
{
	if (110 <= note.value && note.value <= 112)
	{
		m_ext.type = DrumType_Enum::FOURLANE_PRO;
		return m_values.m_subTracks == 31;
	}
	return false;
}
