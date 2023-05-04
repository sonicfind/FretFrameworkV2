#pragma once
#include "InstrumentalTrack.h"
#include "Notes/GuitarNote.h"

template<>
void InstrumentalTrack<GuitarNote<5>, true>::load_V1(size_t diff, ChtFileReader& reader);

template<>
void InstrumentalTrack<GuitarNote<6>, true>::load_V1(size_t diff, ChtFileReader& reader);
