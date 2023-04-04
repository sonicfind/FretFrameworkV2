#include "BCHFileWriter.h"
#include <assert.h>

void BCHFileWriter::writeHeaderTrack(uint32_t tickRate)
{
	writeTrackHeader("BThd");
	write(tickRate);
	writeLengthOfSection<0>();
}

void BCHFileWriter::writeSyncTrack()
{
	writeTrackHeader("BTsn");
	writeLengthOfSection<0>();
}

void BCHFileWriter::writeEventTrack()
{
	writeTrackHeader("BTev");
	writeLengthOfSection<0>();
}

void BCHFileWriter::writeNoteTrack(size_t index)
{
	if (index < 9)
		writeTrackHeader("BTin");
	else if (index < 11)
		writeTrackHeader("BTvc");
	else // Possible Pro Instrument Tag
		writeTrackHeader("BTin");
	write(index, 1);
	writeLengthOfSection<0>();
}

void BCHFileWriter::writeDifficultyTrack(size_t index)
{
	writeTrackHeader("BTdf");
	write(index, 1);
	writeLengthOfSection<0>();
}

void BCHFileWriter::writeAnimationTrack()
{
	writeTrackHeader("BTam");
	writeLengthOfSection<0>();
}

void BCHFileWriter::finishTrack()
{
	writeLengthOfSection<4>();
	m_trackPositions.pop_back();
}

void BCHFileWriter::startEvent(uint32_t position, ChartEvent ev)
{
	assert(ChartEvent::BPM <= ev && ev <= ChartEvent::VOCAL_PERCUSSION);

	writeWebType(position - m_position);
	m_position = position;
	write((unsigned char)ev);
}

void BCHFileWriter::finishEvent()
{
	writeWebType((uint32_t)m_event.size());
	write(m_event.data(), m_event.size());
	m_event.clear();
}

void BCHFileWriter::writeSingleNote(const std::pair<size_t, uint32_t>& note)
{
	assert(note.first < 128);
	if (note.second >= 20)
	{
		append(unsigned char(note.first + 128));
		appendWebType(note.second);
	}
	else
		append((unsigned char)note.first);
}

void BCHFileWriter::writeMultiNote(const std::vector<std::pair<size_t, uint32_t>>& notes)
{
	append((unsigned char)notes.size());
	for (const auto& note : notes)
		writeSingleNote(note);
}

void BCHFileWriter::writeSingleNoteMods(const std::vector<char>& mods)
{
	if (mods.empty())
		return;

	append((unsigned char)mods.size());
	for (const char mod : mods)
		append(mod);
}

void BCHFileWriter::writeMultiNoteMods(const std::vector<std::pair<char, size_t>>& mods)
{
	append((unsigned char)mods.size());
	for (const auto& mod : mods)
	{
		if (mod.second != SIZE_MAX)
			append(mod.first);
		else
		{
			append(char(mod.first | 128));
			append((unsigned char)mod.second);
		}
	}
}

void BCHFileWriter::writeText(std::string_view str)
{
	append(str.data(), str.size());
}

void BCHFileWriter::writeSpecialPhrase(const SpecialPhrase& phrase)
{
	append((unsigned char)phrase.getType());
	appendWebType(phrase.getDuration());
}

void BCHFileWriter::writePitch(Pitch<-1, 9> pitch)
{
	append(pitch.getBinaryValue());
}

void BCHFileWriter::writeLyric(std::pair<size_t, std::string_view> lyric)
{
	append((unsigned char)lyric.first);
	appendWebType((uint32_t)lyric.second.size());
	append(lyric.second.data(), lyric.second.size());
}

void BCHFileWriter::writePitchAndDuration(const std::pair<Pitch<-1, 9>, uint32_t>& note)
{
	writePitch(note.first);
	if (note.second > 0)
		appendWebType(note.second);
}

void BCHFileWriter::writeNoteName(NoteName note, PitchType type)
{
	assert(NoteName::C <= note && note <= NoteName::B);
	append(static_cast<unsigned char>(note));
}

void BCHFileWriter::writeLeftHand(size_t position)
{
	append(position);
}

void BCHFileWriter::writeMicrosPerQuarter(uint32_t micros)
{
	append(micros);
}

void BCHFileWriter::writeTimeSig(TimeSig timeSig)
{
	append(timeSig);
}

void BCHFileWriter::writeAnchor(uint32_t anchor)
{
	append(anchor);
}

void BCHFileWriter::writeWebType(uint32_t value)
{
	if (value < 254)
		write(value, 1);
	else
	{
		bool is32 = value > UINT16_MAX;
		write<char>(254 + is32);
		write(value, 2 + 2ULL * is32);
	}
}

void BCHFileWriter::appendWebType(uint32_t value)
{
	if (value < 254)
		append(value, 1);
	else
	{
		bool is32 = value > UINT16_MAX;
		append<char>(254 + is32);
		append(value, 2 + 2ULL * is32);
	}
}

void BCHFileWriter::writeTrackHeader(const char(&tag)[5])
{
	std::pair<std::streampos, std::streampos> positions;
	writeTag(tag);
	positions.first = tell();
	write(0);
	write(0);
	positions.second = tell();
	m_trackPositions.push_back(positions);
	m_position = 0;
}
