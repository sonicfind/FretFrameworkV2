#pragma once
#include <stdint.h>
#include <string>
#include "Sustained.h"
#include "Pitch.h"

using VocalPitch = Pitch<2, 6>;
struct Vocal
{
	std::u32string lyric;
	VocalPitch pitch;

	Vocal() = default;
	Vocal(std::u32string&& _lyric);
	bool isPlayable() const noexcept { return pitch.isActive(); }
};
