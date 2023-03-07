#include "VocalNote.h"
#include "../Unicode/UnicodeString.h"

bool Vocal::set(char pitch, uint32_t duration)
{
	if (!setPitch(pitch))
		return false;

	setDuration(duration);
	return true;
}

bool Vocal::setPitch(char pitch)
{
	if (pitch < 0)
		return false;

	m_pitch = pitch;
	return true;
}

void Vocal::setDuration(uint32_t duration)
{
	m_duration = duration;
}

void Vocal::setLyric(std::string_view lyric)
{
	m_lyric = UnicodeString::strToU32(lyric);
}

void Vocal::setLyric(const std::u32string& lyric)
{
	m_lyric = lyric;
}
