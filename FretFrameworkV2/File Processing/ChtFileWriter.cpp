#include "ChtFileWriter.h"
#include "ChtConstants.h"
#include <assert.h>

void ChtFileWriter::writeHeaderTrack(uint32_t tickRate)
{
	writeTrack("[Song]");
	m_file << "\tFileVersion = 2\n";
	m_file << "\tResolution = " << tickRate << '\n';
}

void ChtFileWriter::writeSyncTrack()
{
	writeTrack("[SyncTrack]");
}

void ChtFileWriter::writeEventTrack()
{
	writeTrack("[Events]");
}

void ChtFileWriter::writeInstrumentTrack(size_t index)
{
	writeTrack(g_INSTRUMENTTRACKS[index]);
}

void ChtFileWriter::writeVocalTrack(size_t index)
{
	writeTrack(g_VOCALTRACKS[index]);
}

void ChtFileWriter::writeDifficultyTrack(size_t index)
{
	writeTrack(g_DIFFICULTIES[index]);
}

void ChtFileWriter::writeAnimationTrack()
{
	writeTrack("[Animation]");
}

void ChtFileWriter::writeTrack(std::string_view name)
{
	m_file << m_tabs << name << "\n" << m_tabs << "{\n";
	m_tabs += "\t";
}

void ChtFileWriter::finishTrack()
{
	m_tabs.pop_back();
	m_file << m_tabs << "}" << std::endl;
}

void ChtFileWriter::startEvent(uint32_t position, ChartEvent ev)
{
	static constexpr std::string_view EVENTS[] = { "B", "TS", "A", "E", "SE", "N", "C", "M", "S", "L", "V", "P" };
	assert(ChartEvent::BPM <= ev && ev <= ChartEvent::VOCAL_PERCUSSION);
	m_file << m_tabs << position << " = " << EVENTS[static_cast<size_t>(ev)];
}

void ChtFileWriter::finishEvent()
{
	m_file << '\n';
}

void ChtFileWriter::writeSingleNote(const std::pair<size_t, uint32_t>& note)
{
	write(note.first);
	if (note.second >= 20)
		m_file << '~' << note.second;
}

void ChtFileWriter::writeMultiNote(const std::vector<std::pair<size_t, uint32_t>>& notes)
{
	write(notes.size());
	for (const auto& note : notes)
		writeSingleNote(note);
}

void ChtFileWriter::writeSingleNoteMods(const std::vector<char>& mods)
{
	if (mods.empty())
		return;

	write(mods.size());
	for (const char mod : mods)
		write(mod);
}

void ChtFileWriter::writeMultiNoteMods(const std::vector<std::pair<char, size_t>>& mods)
{
	write(mods.size());
	for (const auto& mod : mods)
	{
		write(mod.first);
		if (mod.second < SIZE_MAX)
			write(mod.second);
	}
}

void ChtFileWriter::writeText(std::string_view str)
{
	m_file << " \"" << str << "\"";
}

void ChtFileWriter::writeSpecialPhrase(const SpecialPhrase& phrase)
{
	write(static_cast<uint32_t>(phrase.getType()));
	write(phrase.getDuration());
}

void ChtFileWriter::writeLyric(std::pair<size_t, std::string_view> lyric)
{
	write(lyric.first);
	writeText(lyric.second);
}

void ChtFileWriter::writePitchAndDuration(const std::pair<char, uint32_t>& note)
{
	write<uint32_t>(note.first);
	write(note.second);
}

void ChtFileWriter::writeMicrosPerQuarter(uint32_t micros)
{
	write(g_TEMPO_FACTOR / micros);
}

void ChtFileWriter::writeTimeSig(TimeSig timeSig)
{
	write<uint32_t>(timeSig.numerator);
	if (!timeSig.hasDenominator() && !timeSig.hasMetronome() && !timeSig.has32nds())
		return;

	write<uint32_t>(timeSig.denominator);
	if (!timeSig.hasMetronome() && !timeSig.has32nds())
		return;

	write<uint32_t>(timeSig.metronome);
	if (!timeSig.has32nds())
		return;

	write<uint32_t>(timeSig.num32nds);
}

void ChtFileWriter::writeAnchor(uint32_t anchor)
{
	write(anchor);
}
