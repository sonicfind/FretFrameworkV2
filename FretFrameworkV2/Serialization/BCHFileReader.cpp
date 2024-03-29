#include "BCHFileReader.h"
#include <assert.h>

BCHFileReader::BCHFileReader(const std::filesystem::path& path) : BufferedBinaryReader(path)
{
	parseHeader();
}

BCHFileReader::BCHFileReader(const LoadedFile& file) : BufferedBinaryReader(file)
{
	parseHeader();
}

void BCHFileReader::parseHeader()
{
	if (!validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	m_tickrate = extract<uint32_t>();
	m_nextTracks.pop_back();
	m_ends.pop_back();
}

bool BCHFileReader::isStartOfTrack() const
{
	return strncmp(m_currentPosition, "BT", 2) == 0;
}

bool BCHFileReader::validateHeaderTrack()
{
	return validateTrack("BThd");
}

bool BCHFileReader::validateSyncTrack()
{
	return validateTrack("BTsn");
}

bool BCHFileReader::validateEventTrack()
{
	return validateTrack("BTev");
}

bool BCHFileReader::validateNoteTrack()
{
	if (!validateTrack("BTin") && !validateTrack("BTvc"))
		return false;

	m_noteTrackID = extract<unsigned char>();
	return true;
}

bool BCHFileReader::validateDifficultyTrack()
{
	if (!validateTrack("BTdf"))
		return false;

	m_difficulty = extract<unsigned char>();
	return true;
}

bool BCHFileReader::validateAnimationTrack()
{
	return validateTrack("BTam");
}

[[nodiscard]] bool BCHFileReader::validateTrack(const char(&str)[5])
{
	if (!checkTag(str))
		return false;

	enterSection();
	m_nextTracks.push_back(m_ends.back());
	m_tickPosition = 0;
	return true;
}

void BCHFileReader::skipTrack()
{
	if (isStartOfTrack())
	{
		move(4);
		enterSection();
	}
	else
	{
		if (m_ends.back() != m_nextTracks.back())
			m_ends.pop_back();

		m_nextTracks.pop_back();
	}
	exitSection();
}

bool BCHFileReader::isStillCurrentTrack()
{
	if (m_currentPosition != m_nextTracks.back())
		return true;

	m_nextTracks.pop_back();
	m_ends.pop_back();
	return false;
}

std::pair<uint64_t, ChartEvent>  BCHFileReader::parseEvent()
{
	m_tickPosition += extractWebType();

	ChartEvent type = (ChartEvent)extract<unsigned char>();
	if (type > ChartEvent::VOCAL_PERCUSSION)
		type = ChartEvent::UNKNOWN;

	enterSection();
	return { m_tickPosition, type };
}

void BCHFileReader::skipEvent()
{
	m_tickPosition += extractWebType();
	if (!move(1))
		throw std::runtime_error("Track corrupted");

	const uint64_t length = extractWebType();
	if (!move(length))
		throw std::runtime_error("Track corrupted");
}

void BCHFileReader::nextEvent()
{
	exitSection();
}

std::pair<size_t, uint64_t> BCHFileReader::extractSingleNote()
{
	size_t color = extract<unsigned char>();
	uint64_t sustain = 0;
	if (color >= 128)
	{
		color &= 127;
		sustain = extractWebType();
	}
	return { color, sustain };
}

std::vector<std::pair<size_t, uint64_t>> BCHFileReader::extractMultiNote()
{
	std::vector<std::pair<size_t, uint64_t>> notes;
	size_t numNotes = extract<unsigned char>();
	notes.reserve(numNotes);
	for (size_t i = 0; i < numNotes; ++i)
		notes.push_back(extractSingleNote());
	return notes;
}

std::vector<char> BCHFileReader::extractSingleNoteMods()
{
	std::vector<char> mods;
	unsigned char numMods;
	if (extract(numMods))
	{
		mods.reserve(numMods);
		for (unsigned char i = 0; i < numMods; ++i)
			mods.push_back(extract<char>() & 127);
	}
	return mods;
}

std::vector<std::pair<char, size_t>> BCHFileReader::extractMultiNoteMods()
{
	std::vector<std::pair<char, size_t>> mods;
	unsigned char numMods;
	if (extract(numMods))
	{
		mods.reserve(numMods);
		for (unsigned char i = 0; i < numMods; ++i)
		{
			char modifier = extract<char>();
			size_t lane = SIZE_MAX;
			if (modifier < 0)
			{
				lane = extract<unsigned char>();
				modifier &= 127;
			}
			mods.push_back({ modifier, lane });
		}
		
	}
	return mods;
}

std::string_view BCHFileReader::extractText()
{
	return extractString(m_ends.back() - m_currentPosition);
}

SpecialPhrase BCHFileReader::extractSpecialPhrase()
{
	const size_t type = extract<unsigned char>();
	const uint64_t duration = extractWebType();
	return { (SpecialPhraseType)type, duration };
}

Pitch<-1, 9> BCHFileReader::extractPitch()
{
	return { extract<char>(), 0 };
}

NoteName BCHFileReader::extractNoteName()
{
	Pitch pitch = extractPitch();
	return pitch.getNote();
}

std::pair<size_t, std::string_view> BCHFileReader::extractLyric()
{
	const size_t lane = extract<unsigned char>();
	if (lane == 0)
		return { 0, {} };

	return { lane, extractString() };
}

Pitch<-1, 9> BCHFileReader::extractPitchAndDuration()
{
	Pitch<-1, 9> pitch = extractPitch();
	pitch.setLength(extractWebType());
	return pitch;
}

size_t BCHFileReader::extractLeftHand()
{
	return extract<unsigned char>();
}

uint32_t BCHFileReader::extractMicrosPerQuarter()
{
	return extract<uint32_t>();
}

uint64_t BCHFileReader::extractAnchor()
{
	return extractWebType();
}

TimeSig BCHFileReader::extractTimeSig()
{
	return extract<TimeSig>();
}
