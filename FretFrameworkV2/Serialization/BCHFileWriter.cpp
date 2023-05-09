#include "BCHFileWriter.h"
#include <assert.h>

void BCHFileWriter::writeHeaderTrack(uint32_t tickRate)
{
	writeTrackHeader("BThd");
	write(tickRate);
}

void BCHFileWriter::writeSyncTrack()
{
	writeTrackHeader("BTsn");
}

void BCHFileWriter::writeEventTrack()
{
	writeTrackHeader("BTev");
}

void BCHFileWriter::writeNoteTrack(unsigned char index)
{
	if (index < 9)
		writeTrackHeader("BTin");
	else if (index < 11)
		writeTrackHeader("BTvc");
	else // Possible Pro Instrument Tag
		writeTrackHeader("BTin");
	write(index);
}

void BCHFileWriter::writeDifficultyTrack(unsigned char index)
{
	writeTrackHeader("BTdf");
	write(index);
}

void BCHFileWriter::writeAnimationTrack()
{
	writeTrackHeader("BTam");
}

void BCHFileWriter::finishTrack()
{
	writeTrackLength();
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
	writeBuffer();
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
	append((unsigned char)phrase.type);
	appendWebType(phrase.getLength());
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

void BCHFileWriter::writeTrackHeader(const char(&tag)[5])
{
	writeTag(tag);
	write(0);
	m_trackPositions.push_back(tell());
	m_position = 0;
}

void BCHFileWriter::writeTrackLength()
{
	const auto curr = tell();
	seek(m_trackPositions.back() - std::streamoff(4));
	write(uint32_t(curr - m_trackPositions.back()));
	seek(curr);
}
