#pragma once
#include <stdint.h>
#include <string>
#include "Sustained.h"
#include "Pitch.h"

using VocalPitch = Pitch<2, 6>;
class Vocal : public Sustained
{
	std::u32string m_lyric;
	VocalPitch m_pitch;

public:
	bool set(unsigned char pitch, uint32_t length) noexcept;

	template <int min, int max>
	bool set(Pitch<min, max> pitch, uint32_t length) noexcept
	{
		if (!setPitch(pitch))
			return false;

		setLength(length);
		return true;
	}

	template <int min, int max>
	bool setPitch(Pitch<min, max> pitch) noexcept
	{
		return m_pitch.set(pitch);
	}

	void setLyric(std::string_view lyric);
	void setLyric(const std::u32string& lyric);

	void resetPitch() noexcept;

	const std::u32string& getLyric() const noexcept { return m_lyric; }
	std::pair<VocalPitch, uint32_t> getPitchAndDuration() const noexcept { return { m_pitch, m_length }; }
	VocalPitch getPitch() const noexcept { return m_pitch; }
	bool isPlayable() const noexcept { return isActive() || m_pitch.isActive(); }
};
