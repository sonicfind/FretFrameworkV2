#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "TimeSig.h"
#include "MicrosPerQuarter.h"

struct SyncTrack
{
	SimpleFlatMap<std::pair<MicrosPerQuarter, uint64_t>> tempoMarkers;
	SimpleFlatMap<TimeSig> timeSigs;
};
