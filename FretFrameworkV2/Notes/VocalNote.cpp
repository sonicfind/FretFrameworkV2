#include "VocalNote.h"
#include "Types/UnicodeString.h"

bool Vocal::set(unsigned char pitch, uint32_t duration) noexcept
{
	return set(Pitch<-1, 9>(pitch), duration);
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
