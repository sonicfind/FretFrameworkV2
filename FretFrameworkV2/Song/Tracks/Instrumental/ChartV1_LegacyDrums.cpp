#include "ChartV1_LegacyDrums.h"

ChartV1::V1Loader<DrumNote_Legacy>::V1Loader(DrumType_Enum type) : m_type(type) {}

bool ChartV1::V1Loader<DrumNote_Legacy>::addNote(SimpleFlatMap<DrumNote_Legacy>& notes, uint32_t position, std::pair<size_t, uint32_t> note)
{
	if (!notes.get_or_emplace_back(position).set_V1(note.first, note.second))
		return false;

	if (m_type == DrumType_Enum::LEGACY)
		m_type = DrumNote_Legacy::EvaluateDrumType(note.first);
	return true;
}
