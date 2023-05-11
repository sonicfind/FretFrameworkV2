#include "VocalNote.h"

Vocal::Vocal(std::u32string&& _lyric) : lyric(std::move(_lyric)) {}
