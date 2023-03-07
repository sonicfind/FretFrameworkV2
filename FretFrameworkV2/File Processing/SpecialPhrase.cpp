#include "SpecialPhrase.h"
//#include "Variable Types/WebType.h"
//
//void SpecialPhrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
//{
//	outFile << tabs << position << " = S " << m_type << ' ' << m_duration << '\n';
//}
//
//void SpecialPhrase::save_bch(std::fstream& outFile) const
//{
//	static char buffer[7] = { 5, 0, 0, 0, 0, 0, 0 };
//	static char* start = buffer + 3;
//
//	char* current = start;
//	buffer[2] = (char)m_type;
//	WebType::copyToBuffer(m_duration, current);
//	buffer[1] = (char)(current - start + 1);
//	outFile.write(buffer, 2ULL + buffer[1]);
//}

char SpecialPhrase::getMidiNote() const
{
	static constexpr char MIDI_VALUES[] = { 116, -1, 103, 126, 127, 105, 0, 106, 1, 105, 106 };
	return MIDI_VALUES[static_cast<size_t>(m_type)];
}

void SpecialPhrase::operator*=(float multiplier)
{
	m_duration = uint32_t(m_duration * multiplier);
}
