#include "Legacy_DrumTrack.h"

Legacy_DrumTrack::Legacy_DrumTrack(DrumType_Enum type) : m_type(type) {}
Legacy_DrumTrack::Legacy_DrumTrack(MidiFileReader& reader)
{
	InstrumentalTrack<DrumNote_Legacy>::load(reader);
	for (size_t i = 0; i < 4; ++i)
	{
		m_type = evaluateDrumType(i);
		if (m_type != DrumType_Enum::LEGACY)
			break;
	}
}

void Legacy_DrumTrack::load_V1(size_t diff, ChtFileReader& reader)
{
	InstrumentalTrack<DrumNote_Legacy>::load_V1(diff, reader);
	m_type = evaluateDrumType(diff);
}

[[nodiscard]] DrumType_Enum Legacy_DrumTrack::evaluateDrumType(size_t index)
{
	for (const auto& note : m_difficulties[index].m_notes)
	{
		DrumType_Enum type = note->evaluateDrumType();
		if (type != DrumType_Enum::LEGACY)
			return type;
	}
	return DrumType_Enum::LEGACY;
}

template <>
void InstrumentalTrack<DrumNote_Legacy>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENABLE_CHART_DYNAMICS]")
		tracker.ext.enableDynamics = true;
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}
