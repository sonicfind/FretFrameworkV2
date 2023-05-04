#pragma once
#include <stdint.h>
#include <string>
#include "Pitch.h"

using VocalPitch = Pitch<2, 6>;
class Vocal
{
	std::u32string m_lyric;
	VocalPitch m_pitch;
	uint32_t m_duration = 0;

public:
	bool set(unsigned char pitch, uint32_t duration) noexcept;

	template <int min, int max>
	bool set(Pitch<min, max> pitch, uint32_t duration) noexcept
	{
		if (!setPitch(pitch))
			return false;

		setDuration(duration);
		return true;
	}

	template <int min, int max>
	bool setPitch(Pitch<min, max> pitch) noexcept
	{
		return m_pitch.set(pitch);
	}

	void setDuration(uint32_t duration) noexcept;

	void setLyric(std::string_view lyric);
	void setLyric(const std::u32string& lyric);

	void resetPitch() noexcept;

	void operator*=(float multiplier) noexcept { m_duration = uint32_t(m_duration * multiplier); }

	const std::u32string& getLyric() const noexcept { return m_lyric; }
	std::pair<VocalPitch, uint32_t> getPitchAndDuration() const noexcept { return { m_pitch, m_duration }; }
	VocalPitch getPitch() const noexcept { return m_pitch; }
	uint32_t getDuration() const noexcept { return m_duration; }
	bool isPlayable() const noexcept { return m_duration > 0 || m_pitch.isActive(); }
};
