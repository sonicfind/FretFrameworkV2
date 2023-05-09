#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Types/UnicodeString.h"

struct SongEvents
{
	SimpleFlatMap<UnicodeString> sections;
	SimpleFlatMap<std::vector<std::u32string>> globals;
};
