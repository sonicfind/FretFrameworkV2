#include "VocalNote.h"
#include "../Unicode/UnicodeString.h"

bool Vocal::set(VocalPitch pitch, uint32_t duration) noexcept
{
	if (!setPitch(pitch))
		return false;

	setDuration(duration);
	return true;
}

bool Vocal::setPitch(VocalPitch pitch) noexcept
{
	return m_pitch.set(pitch);
}

void Vocal::setDuration(uint32_t duration) noexcept
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

void Vocal::resetPitch() noexcept
{
	m_pitch.disable();
}
