#include "DrumScan_Legacy_Midi.h"

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

DrumScan_Legacy_Midi::DrumScan_Legacy_Midi(MidiFileReader& reader) : DrumScan_Legacy(Midi_Scanner<DrumNote_Legacy>(m_values, reader).m_ext.type) {}
