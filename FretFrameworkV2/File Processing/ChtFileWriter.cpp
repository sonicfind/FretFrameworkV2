#include "ChtFileWriter.h"
#include "ChtConstants.h"
#include <assert.h>

void ChtFileWriter::setPitchMode(PitchWriteMode mode) { m_pitchMode = mode; }

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

void ChtFileWriter::writeNoteTrack(size_t index)
{
	writeTrack(g_NOTETRACKS[index]);
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
	static constexpr std::string_view EVENTS[] = { "B", "TS", "A", "E", "SE", "N", "C", "M", "S", "L", "V", "VP", "NP", "CP", "R", "LH", "P", "RS" };
	assert(ChartEvent::BPM <= ev && ev <= ChartEvent::RANGE_SHIFT);
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

void ChtFileWriter::writePitch(Pitch<-1, 9> pitch)
{
	static constexpr std::string_view STRINGS[2][12] =
	{
		{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", },
		{ "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B", }
	};

	write(STRINGS[static_cast<size_t>(m_pitchMode)][static_cast<size_t>(pitch.getNote())]);
	m_file << pitch.getOctave();
}

void ChtFileWriter::writeLyric(std::pair<size_t, std::string_view> lyric)
{
	write(lyric.first);
	writeText(lyric.second);
}

void ChtFileWriter::writePitchAndDuration(const std::pair<Pitch<-1, 9>, uint32_t>& note)
{
	writePitch(note.first);
	if (note.second > 0)
		write(note.second);
}

void ChtFileWriter::writeNoteName(NoteName note, PitchType type)
{
	switch (note)
	{
	case NoteName::C_Sharp_Db:
	case NoteName::D_Sharp_Eb:
	case NoteName::F_Sharp_Gb:
	case NoteName::G_Sharp_Ab:
	case NoteName::A_Sharp_Bb:
		if (type == PitchType::Flat)
		{
			static constexpr std::string_view FLATS[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
			write(FLATS[static_cast<size_t>(note)]);
			break;
		}
		__fallthrough;
	case NoteName::C:
	case NoteName::D:
	case NoteName::E:
	case NoteName::F:
	case NoteName::G:
	case NoteName::A:
	case NoteName::B:
	{
		static constexpr std::string_view SHARPS[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		write(SHARPS[static_cast<size_t>(note)]);
		break;
	}
	default:
		assert(false);
	}
}

void ChtFileWriter::writeLeftHand(size_t position)
{
	write(position);
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
