#include "DrumScan_Legacy_Midi.h"

template <>
bool InstrumentalScan_Midi::Midi_Scanner<DrumNote_Legacy>::processExtraValues(ScanValues& values, MidiNote note)
{
	if (110 <= note.value && note.value <= 112)
	{
		m_ext.type = DrumType_Enum::FOURLANE_PRO;
		return values.m_subTracks == 31;
	}
	return false;
}

DrumScan_Legacy_Midi::DrumScan_Legacy_Midi(MidiFileReader& reader)
{
	const auto scanner = InstrumentalScan_Midi::Scan<DrumNote_Legacy>(m_values, reader);
	m_type = scanner.m_ext.type;
}
