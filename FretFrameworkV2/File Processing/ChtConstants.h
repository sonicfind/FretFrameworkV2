#pragma once
#include "CommonChartTypes.h"

static constexpr std::string_view g_NOTETRACKS[] = { "[LeadGuitar]", "[LeadGuitar_GHL]", "[BassGuitar]", "[BassGuitar_GHL]", "[RhythmGuitar]", "[CoopGuitar]", "[Keys]", "[Drums_4Lane]", "[Drums_5Lane]",
                                                     "[Vocals]", "[Harmonies]" };
static constexpr std::string_view g_DIFFICULTIES[] = { "[Easy]", "[Medium]", "[Hard]", "[Expert]", "[BRE]" };
static constexpr uint64_t g_TEMPO_FACTOR = 60000000000;
