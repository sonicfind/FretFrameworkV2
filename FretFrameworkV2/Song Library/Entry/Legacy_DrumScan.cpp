#include "Legacy_DrumScan.h"

bool Legacy_DrumScan::extraTest_V1(size_t note)
{
	m_type = DrumNote_Legacy::EvaluateDrumType(note);
	return m_type != DrumType_Enum::LEGACY;
}

void Legacy_DrumScan::applyExtraValue(Midi_Scanner& scanner)
{
	m_type = scanner.ext.type;
	InstrumentalScan<DrumNote_Legacy>::applyExtraValue(scanner);
}

Legacy_DrumScan::Legacy_DrumScan(DrumType_Enum type) : m_type(type){}
Legacy_DrumScan::Legacy_DrumScan(MidiFileReader& reader)
{
	InstrumentalScan<DrumNote_Legacy>::scan(reader);
}
