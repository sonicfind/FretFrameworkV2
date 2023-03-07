#pragma once
#include <stdint.h>
#include <string>

class Vocal
{
	std::u32string m_lyric;
	char m_pitch = 0;
	uint32_t m_duration = 0;

public:
	bool set(char pitch, uint32_t duration);
	bool setPitch(char pitch);
	void setDuration(uint32_t duration);

	void setLyric(std::string_view lyric);
	void setLyric(const std::u32string& lyric);

	void operator*=(float multiplier) { m_duration = uint32_t(m_duration * multiplier); }

	const std::u32string& getLyric() const { return m_lyric; }
	std::pair<char, uint32_t> getPitchAndDuration() const { return { m_pitch, m_duration }; }
	char getPitch() const { return m_pitch; }
	uint32_t getDuration() const { return m_duration; }
};
