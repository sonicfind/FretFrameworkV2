#include "Midi_Scan_Drums.h"
template <>
void Midi_Scanner_Instrument::Scanner<DrumNote_Legacy>::processExtraValues(MidiNote note)
{
	if (110 <= note.value && note.value <= 112)
		m_scan.setType(DrumType_Enum::FOURLANE_PRO);
}
