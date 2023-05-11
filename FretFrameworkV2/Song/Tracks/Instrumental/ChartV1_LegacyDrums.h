#pragma once
#include "ChartV1.h"
#include "Notes/DrumNote_Legacy.h"

template<>
class ChartV1::V1Loader<DrumNote_Legacy>
{
public:
	V1Loader(DrumType_Enum type);
	[[nodiscard]] DrumType_Enum getDrumType() const noexcept { return m_type; }
	[[nodiscard]] bool addNote(SimpleFlatMap<DrumNote_Legacy>& notes, uint64_t position, std::pair<size_t, uint64_t> note);

private:
	DrumType_Enum m_type;
};
