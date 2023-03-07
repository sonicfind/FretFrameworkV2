#include "SyncValues.h"

void SyncValues::setMicrosPerQuarter(uint32_t micros)
{
	m_microsPerQuarter = micros;
}

void SyncValues::setMicrosPerQuarter(float bpm)
{
	m_microsPerQuarter = uint32_t(BPM_FACTOR / bpm);
}

void SyncValues::setTimeSig(TimeSig timeSig)
{
	m_timeSig = timeSig;
}

void SyncValues::setAnchor(uint32_t anchor)
{
	m_anchorPoint = anchor;
}
