#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "TimeSig.h"
#include "MicrosPerQuarter.h"

struct SyncTrack
{
	SimpleFlatMap<MicrosPerQuarter> tempoMarkers;
	SimpleFlatMap<TimeSig> timeSigs;
};
