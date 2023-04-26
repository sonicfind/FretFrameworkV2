#pragma once
#include "Serialization/CommonChartParser.h"
class ScanTrack
{
public:
	unsigned char m_subTracks = 0;
	unsigned char m_intensity = 0xFF;
};

class BCH_CHT_Scannable
{
public:
	virtual void scan(CommonChartParser* parser) = 0;
};
