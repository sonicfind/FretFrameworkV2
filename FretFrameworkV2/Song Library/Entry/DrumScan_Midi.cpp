#include "DrumScan_Midi.h"

template <>
bool Midi_Scanner<DrumNote<4, true>>::isFinished() const noexcept
{
	return m_values.m_subTracks == 31;
}

template <>
bool Midi_Scanner<DrumNote<5, false>>::isFinished() const noexcept
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

DrumScan_Legacy_Midi::DrumScan_Legacy_Midi(ScanValues& values, MidiFileReader& reader) : DrumScan_Legacy(Midi_Scanner<DrumNote_Legacy>(values, reader).m_ext.type) {}
