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

void BCHFileWriter::writeTrackHeader(const char(&tag)[5])
{
	write(tag, 4);
	startBuffer();
	m_position = 0;
}

void BCHFileWriter::finishTrack()
{
	endBuffer();
}

void BCHFileWriter::startEvent(uint64_t position, ChartEvent ev)
{
	assert(ChartEvent::BPM <= ev && ev <= ChartEvent::VOCAL_PERCUSSION);

	writeWebType(position - m_position);
	m_position = position;
	write((unsigned char)ev);
}

void BCHFileWriter::finishEvent()
{
	endBuffer();
}

void BCHFileWriter::writeSingleNote(const std::pair<size_t, uint64_t>& note)
{
	assert(note.first < 128);
	if (note.second >= 20)
	{
		write(unsigned char(note.first + 128));
		writeWebType(note.second);
	}
	else
		write((unsigned char)note.first);
}

void BCHFileWriter::writeMultiNote(const std::vector<std::pair<size_t, uint64_t>>& notes)
{
	write((unsigned char)notes.size());
	for (const auto& note : notes)
		writeSingleNote(note);
}

void BCHFileWriter::writeSingleNoteMods(const std::vector<char>& mods)
{
	if (mods.empty())
		return;

	write((unsigned char)mods.size());
	for (const char mod : mods)
		write(mod);
}

void BCHFileWriter::writeMultiNoteMods(const std::vector<std::pair<char, size_t>>& mods)
{
	write((unsigned char)mods.size());
	for (const auto& mod : mods)
	{
		if (mod.second != SIZE_MAX)
			write(mod.first);
		else
		{
			write(char(mod.first | 128));
			write((unsigned char)mod.second);
		}
	}
}

void BCHFileWriter::writeText(std::string_view str)
{
	write(str.data(), str.size());
}

void BCHFileWriter::writeSpecialPhrase(const SpecialPhrase& phrase)
{
	write((unsigned char)phrase.type);
	writeWebType(phrase.getLength());
}

void BCHFileWriter::writePitch(Pitch<-1, 9> pitch)
{
	write(pitch.getBinaryValue());
}

void BCHFileWriter::writePitchAndDuration(Pitch<-1, 9> pitch)
{
	writePitch(pitch);
	if (pitch.getLength() > 0)
		writeWebType(pitch.getLength());
}

void BCHFileWriter::writeLyric(std::pair<size_t, std::string_view> lyric)
{
	write((unsigned char)lyric.first);
	writeWebType(lyric.second.size());
	write(lyric.second.data(), lyric.second.size());
}

void BCHFileWriter::writeNoteName(NoteName note, PitchType type)
{
	assert(NoteName::C <= note && note <= NoteName::B);
	write(static_cast<unsigned char>(note));
}

void BCHFileWriter::writeLeftHand(size_t position)
{
	write(position);
}

void BCHFileWriter::writeMicrosPerQuarter(uint32_t micros)
{
	write(micros);
}

void BCHFileWriter::writeAnchor(uint64_t anchor)
{
	writeWebType(anchor);
}

void BCHFileWriter::writeTimeSig(TimeSig timeSig)
{
	write(timeSig);
}
