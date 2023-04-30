#include "BCHFileReader.h"
#include <assert.h>

void BCHFileReader::parseTrackHeader()
{
	uint32_t trackSize = extract_nonvirtual<uint32_t>();
	if (m_currentPosition + trackSize > m_file.end())
		throw std::runtime_error("BCH Track extends past EOF");

	m_nextTracks.push_back(m_currentPosition + trackSize);
}

bool BCHFileReader::testExtractWebType() noexcept
{
	unsigned char val;
	if (!extract(val))
		return false;

	return val < 254 || move(2 + 2ULL * (val == 255));
}

bool BCHFileReader::move(size_t amount)
{
	if (m_currentPosition + amount > m_next)
		return false;

	return FileReader::move(amount);
}

bool BCHFileReader::isStartOfTrack() const
{
	return strncmp(m_currentPosition, "BT", 2) == 0;
}

bool BCHFileReader::validateHeaderTrack()
{
	if (!validateTrack("BThd"))
		return false;

	m_next = m_nextTracks.back();
	return true;
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

	m_noteTrackID = extract_nonvirtual<unsigned char>();
	return true;
}

bool BCHFileReader::validateDifficultyTrack()
{
	if (!validateTrack("BTdf"))
		return false;

	m_difficulty = extract_nonvirtual<unsigned char>();
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

	parseTrackHeader();
	m_tickPosition = 0;
	return true;
}

void BCHFileReader::skipTrack()
{
	if (isStartOfTrack())
	{
		FileReader::move(4);
		parseTrackHeader();
	}
	m_currentPosition = m_next = m_nextTracks.back();
	m_nextTracks.pop_back();
}

bool BCHFileReader::isStillCurrentTrack()
{
	if (m_currentPosition != m_nextTracks.back())
		return true;

	m_nextTracks.pop_back();
	return false;
}

std::pair<uint32_t, ChartEvent>  BCHFileReader::parseEvent()
{
	m_tickPosition += extractWebType<false>();

	ChartEvent type = (ChartEvent)extract_nonvirtual<unsigned char>();
	if (type > ChartEvent::VOCAL_PERCUSSION)
		type = ChartEvent::UNKNOWN;

	const uint32_t length = extractWebType<false>();
	m_next = m_currentPosition + length;

	if (m_next > m_nextTracks.back())
		throw std::runtime_error("Invalid length for BCH Track event");

	return { m_tickPosition, type };
}

void BCHFileReader::nextEvent()
{
	m_currentPosition = m_next;
}

std::pair<size_t, uint32_t> BCHFileReader::extractSingleNote()
{
	size_t color = extract<unsigned char>();
	uint32_t sustain = 0;
	if (color >= 128)
	{
		color &= 127;
		sustain = extractWebType();
	}
	return { color, sustain };
}

std::vector<std::pair<size_t, uint32_t>> BCHFileReader::extractMultiNote()
{
	std::vector<std::pair<size_t, uint32_t>> notes;
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
			uint32_t lane = UINT32_MAX;
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

std::string_view BCHFileReader::extractText(size_t length)
{
	std::string_view str(m_currentPosition, length);
	if (!move(length))
		throw std::runtime_error("length of text invalid");
	return str;
}

std::string_view BCHFileReader::extractText()
{
	return extractText(m_next - m_currentPosition);
}

SpecialPhrase BCHFileReader::extractSpecialPhrase()
{
	uint32_t type = extract<unsigned char>();
	uint32_t duration = extractWebType();
	return { (SpecialPhraseType)type, duration };
}

Pitch<-1, 9> BCHFileReader::extractPitch()
{
	return { extract<char>() };
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

	const uint32_t length = extractWebType();
	return { lane, extractText(length) };
}

std::pair<Pitch<-1, 9>, uint32_t> BCHFileReader::extractPitchAndDuration()
{
	Pitch<-1, 9> pitch = extractPitch();
	uint32_t duration = 0;
	extractWebType(duration);
	return { pitch, duration };
}

size_t BCHFileReader::extractLeftHand()
{
	return extract<unsigned char>();
}

uint32_t BCHFileReader::extractMicrosPerQuarter()
{
	return extract<uint32_t>();
}

TimeSig BCHFileReader::extractTimeSig()
{
	return extract<TimeSig>();
}
