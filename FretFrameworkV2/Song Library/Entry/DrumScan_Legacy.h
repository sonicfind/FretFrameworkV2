#pragma once
#include "DrumScan.h"
#include "Notes/DrumNote_Legacy.h"

template<>
class InstrumentScan<DrumNote_Legacy> : public InstrumentScan<DrumNote<DrumPad_Pro, 5>>
{
	DrumType_Enum m_type;
public:
	InstrumentScan(DrumType_Enum type);
	void setType(DrumType_Enum type);
	DrumType_Enum getType() const noexcept { return m_type; }
	bool isComplete() const noexcept { return m_subTracks == 31 && m_type != DrumType_Enum::LEGACY; }
};
