#include "ChtFileReader.h"
#include "ChtConstants.h"
#include "Notes/NoteName.h"

static constexpr EventCombo g_TEMPO{ "B",  ChartEvent::BPM };
static constexpr EventCombo g_TIMESIG{ "TS", ChartEvent::TIME_SIG };
static constexpr EventCombo g_ANCHOR{ "A",  ChartEvent::ANCHOR };
static constexpr EventCombo g_TEXT{ "E",  ChartEvent::EVENT };
static constexpr EventCombo g_SECTION{ "SE", ChartEvent::SECTION };
static constexpr EventCombo g_NOTE{ "N",  ChartEvent::NOTE };
static constexpr EventCombo g_MULTI{ "C",  ChartEvent::MULTI_NOTE };
static constexpr EventCombo g_MODIFIER{ "M",  ChartEvent::MODIFIER };
static constexpr EventCombo g_SPECIAL{ "S",  ChartEvent::SPECIAL };

static constexpr EventCombo g_LYRIC{ "L",  ChartEvent::LYRIC };
static constexpr EventCombo g_VOCAL{ "V",  ChartEvent::VOCAL };
static constexpr EventCombo g_PERC{ "VP",  ChartEvent::VOCAL_PERCUSSION };

static constexpr EventCombo g_NOTE_PRO{ "NP",  ChartEvent::NOTE_PRO };
static constexpr EventCombo g_MULTI_PRO{ "CP",  ChartEvent::MUTLI_NOTE_PRO };
static constexpr EventCombo g_ROOT{ "R",  ChartEvent::ROOT };
static constexpr EventCombo g_LEFT_HAND{ "LH",  ChartEvent::LEFT_HAND };

static constexpr EventCombo g_PITCH{ "P",  ChartEvent::PITCH };
static constexpr EventCombo g_RANGE_SHIFT{ "RS",  ChartEvent::RANGE_SHIFT };

static const std::vector<EventCombo> g_ALLEVENTS = { g_TEMPO, g_TIMESIG, g_ANCHOR, g_TEXT, g_SECTION, g_NOTE, g_MULTI, g_MODIFIER,  g_SPECIAL,
                                                     g_LYRIC, g_VOCAL,  g_PERC, g_NOTE_PRO, g_MULTI_PRO, g_ROOT, g_LEFT_HAND, g_PITCH, g_RANGE_SHIFT };

const std::vector<EventCombo> g_validTypes[] =
{
	{ g_TEMPO,       g_TIMESIG,   g_ANCHOR }, // TempoMap
	{ g_TEXT,        g_SECTION },             // Events
	{ g_SPECIAL,     g_TEXT },                // Instruments
	{ g_NOTE,        g_MULTI,     g_MODIFIER, g_SPECIAL,   g_TEXT }, // Difficulties
	{ g_LYRIC,       g_VOCAL,     g_PERC,     g_SPECIAL,   g_TEXT }, // Vocals
	{ g_ROOT,        g_LEFT_HAND, g_MODIFIER, g_SPECIAL,   g_TEXT }, // Pro Guitar Instrument
	{ g_RANGE_SHIFT, g_SPECIAL,   g_TEXT },                          // Pro Keys Instrument
	{ g_NOTE_PRO,    g_MULTI_PRO, g_MODIFIER, g_SPECIAL,   g_TEXT }, // Pro Guitar Difficulty
	{ g_PITCH,       g_SPECIAL,   g_TEXT },                          // Pro Keys Difficulty
};

bool ChtFileReader::doesStringMatch(std::string_view str) const
{
	return strncmp((const char*)m_currentPosition, str.data(), str.size()) == 0;
}

bool ChtFileReader::isStartOfTrack() const
{
	return *m_currentPosition == '[';
}

bool ChtFileReader::validateHeaderTrack()
{
	if (!validateTrack("[Song]"))
		return false;

	m_eventSets.push_back(nullptr);
	return true;
}

bool ChtFileReader::validateSyncTrack()
{
	if (!validateTrack("[SyncTrack]"))
		return false;

	m_eventSets.push_back(g_validTypes);
	return true;
}

bool ChtFileReader::validateEventTrack()
{
	if (!validateTrack("[Events]"))
		return false;

	m_eventSets.push_back(g_validTypes + 1);
	return true;
}

bool ChtFileReader::validateNoteTrack()
{
	for (size_t index = 0; index < std::size(g_NOTETRACKS); ++index)
		if (validateTrack(g_NOTETRACKS[index]))
		{
			if (index < 9)
				m_eventSets.push_back(g_validTypes + 2);
			else if (index < 11)
				m_eventSets.push_back(g_validTypes + 4);
			else
				m_eventSets.push_back(g_validTypes + 5 + (index > 11));
			m_noteTrackID = index;
			return true;
		}
	return false;
}

bool ChtFileReader::validateDifficultyTrack()
{
	for (size_t index = std::size(g_DIFFICULTIES); index > 0;)
	{
		--index;
		if (validateTrack(g_DIFFICULTIES[index]))
		{
			if (m_noteTrackID < 9)
				m_eventSets.push_back(g_validTypes + 3);
			else
				m_eventSets.push_back(g_validTypes + 7 + (m_noteTrackID > 9));
			m_difficulty = index;
			return true;
		}
	}
	return false;
}

bool ChtFileReader::validateAnimationTrack()
{
	if (!validateTrack("[Animation]"))
		return false;

	m_eventSets.push_back(&g_ALLEVENTS);
	return true;
}

bool ChtFileReader::validateTrack(std::string_view str)
{
	if (!doesStringMatch(str))
		return false;

	gotoNextLine();
	m_tickPosition = 0;
	return true;
}

void ChtFileReader::skipTrack()
{
	gotoNextLine();
	uint32_t scopeLevel = 1;
	size_t length = strcspn(m_currentPosition, "[}");
	while (m_currentPosition + length != m_file.end())
	{
		size_t index = length - 1;
		char point = m_currentPosition[index];
		while (index > 0 && point <= 32 && point != '\n')
			point = m_currentPosition[--index];

		m_currentPosition += length;
		if (point == '\n')
		{
			if (*m_currentPosition == '}')
			{
				if (scopeLevel == 1)
				{
					setNextPointer();
					gotoNextLine();
					return;
				}
				else
					--scopeLevel;
			}
			else
				++scopeLevel;
		}

		length = strcspn(++m_currentPosition, "[}");
	}

	m_next = m_currentPosition = m_file.end();
}

bool ChtFileReader::isStillCurrentTrack()
{
	unsigned char ch = *m_currentPosition;
	if (ch == '}')
	{
		gotoNextLine();
		m_eventSets.pop_back();
		return false;
	}

	return ch != 0;
}

std::pair<uint64_t, ChartEvent> ChtFileReader::parseEvent()
{
	const uint64_t position = extract<uint64_t>();
	if (position < m_tickPosition)
		throw std::runtime_error(".Cht/.Chart position out of order (previous:  " + std::to_string(m_tickPosition) + ')');

	m_tickPosition = position;

	const char* const start = m_currentPosition;
	while (('A' <= *m_currentPosition && *m_currentPosition <= 'Z') || ('a' <= *m_currentPosition && *m_currentPosition <= 'z'))
		++m_currentPosition;

	std::string_view type(start, m_currentPosition);
	for (const auto& combo : *m_eventSets.back())
		if (type == combo.first)
		{
			skipWhiteSpace();
			return { position, combo.second };
		}

	for (const auto& combo : g_ALLEVENTS)
		if (type == combo.first)
		{
			skipWhiteSpace();
			return { position, combo.second };
		}
	return { position, ChartEvent::UNKNOWN };
}

void ChtFileReader::nextEvent()
{
	gotoNextLine();
}

std::pair<size_t, uint64_t> ChtFileReader::extractSingleNote()
{
	const size_t color = extract<uint32_t>();
	uint64_t sustain = 0;

	if (*m_currentPosition == '~')
	{
		m_currentPosition++;
		sustain = extract<uint64_t>();
	}
	return { color, sustain };
}

std::vector<std::pair<size_t, uint64_t>> ChtFileReader::extractMultiNote()
{
	std::vector<std::pair<size_t, uint64_t>> notes;
	size_t numNotes = extract<uint64_t>();
	notes.reserve(numNotes);
	for (size_t i = 0; i < numNotes; ++i)
		notes.push_back(extractSingleNote());
	return notes;
}

std::vector<char> ChtFileReader::extractSingleNoteMods()
{
	std::vector<char> mods;
	uint32_t numMods;
	if (extract(numMods))
	{
		mods.reserve(numMods);
		for (uint32_t i = 0; i < numMods; ++i)
			mods.push_back(extract<char>() & 127);
	}
	return mods;
}

std::vector<std::pair<char, size_t>> ChtFileReader::extractMultiNoteMods()
{
	std::vector<std::pair<char, size_t>> mods;
	unsigned char numMods;
	if (extract(numMods))
	{
		mods.reserve(numMods);
		for (unsigned char i = 0; i < numMods; ++i)
		{
			char modifier = extract<char>() & 127;
			uint32_t lane = UINT32_MAX;
			extract(lane);
			mods.push_back({ modifier, lane });
		}
	}
	return mods;
}

std::string_view ChtFileReader::extractText()
{
	return TxtFileReader::extractText();
}

SpecialPhrase ChtFileReader::extractSpecialPhrase()
{
	uint32_t type = extract<uint32_t>();
	uint32_t duration = extract<uint32_t>();
	return { (SpecialPhraseType)type, duration };
}

NoteName ChtFileReader::extractNoteName()
{
	static constexpr NoteName NOTES[] = { NoteName::A, NoteName::B, NoteName::C, NoteName::D, NoteName::E, NoteName::F, NoteName::G, };
	unsigned char index = (*m_currentPosition & (char)~32) - 'A';
	if (index >= std::size(NOTES))
		throw std::runtime_error("String is not a valid pitchname");

	NoteName note = NOTES[index];
	++m_currentPosition;
	switch (*m_currentPosition)
	{
	case '#':
		if (note != NoteName::B)
			note = (NoteName)((int)note + 1);
		else
			note = NoteName::C;
		m_currentPosition++;
		break;
	case 'b':
		if (note != NoteName::C)
			note = (NoteName)((int)note - 1);
		else
			note = NoteName::B;
		m_currentPosition++;
		break;
	}
	return note;
}

Pitch<-1, 9> ChtFileReader::extractPitch()
{
	NoteName note = extractNoteName();
	int32_t octave = -1;
	if (!extract(octave))
		skipWhiteSpace();
	return { note, octave };
}

std::pair<size_t, std::string_view> ChtFileReader::extractLyric()
{
	size_t lane = extract<uint32_t>();
	if (lane == 0)
		return { 0, {} };

	auto boundaries = [&]() -> std::pair<const char*, const char*>
	{
		const size_t offset = size_t(1) + (m_next != m_file.end());
		if (*m_currentPosition == '\"')
		{
			for (const char* test = m_currentPosition + 1; test < m_next; ++test)
			{
				if (*test == '\"')
					return { m_currentPosition + 1, test };

				if (*test == '\\')
					++test;
			}
		}
		return {};
	}();

	if (!boundaries.first)
		throw std::runtime_error("Lyrics must be surrounded by quotation marks");

	std::string_view str(boundaries.first, boundaries.second);
	m_currentPosition = boundaries.second + 1;
	skipWhiteSpace();
	return { lane, str };
}

std::pair<Pitch<-1, 9>, uint64_t> ChtFileReader::extractPitchAndDuration()
{
	Pitch<-1, 9> pitch = extractPitch();
	uint64_t duration = 0;
	extract(duration);
	return { pitch, duration };
}

size_t ChtFileReader::extractLeftHand()
{
	return extract<uint32_t>();
}

uint32_t ChtFileReader::extractMicrosPerQuarter()
{
	long double bpm1000 = extract<uint32_t>();
	return (uint32_t)round(g_TEMPO_FACTOR / bpm1000);
}

TimeSig ChtFileReader::extractTimeSig()
{
	uint32_t numerator = extract<uint32_t>();
	uint32_t denom = 255, metro = 0, n32nds = 0;
	if (extract(denom))
		if (extract(metro))
			extract(n32nds);

	return { (unsigned char)numerator, (unsigned char)denom, (unsigned char)metro, (unsigned char)n32nds };
}

std::pair<size_t, uint32_t> ChtFileReader::extractColorAndSustain_V1()
{
	const uint32_t color = extract<uint32_t>();
	uint32_t sustain = extract<uint32_t>();
	return { color, sustain };
}

bool ChtFileReader::checkDifficulty_V1()
{
	static constexpr std::string_view DIFFICULTIES[] = { "[Easy", "[Medium", "[Hard", "[Expert" };
	for (int diff = 3; diff >= 0; --diff)
		if (doesStringMatch(DIFFICULTIES[diff]))
		{
			m_difficulty = diff;
			m_currentPosition += DIFFICULTIES[diff].size();
			return true;
		}
	return false;
}

ChtFileReader::NoteTracks_V1 ChtFileReader::checkTrack_V1()
{
	static constexpr std::pair<std::string_view, NoteTracks_V1> TRACKS[] =
	{
		{ "Single]", Single },
		{ "DoubleGuitar]", DoubleGuitar },
		{ "DoubleBass]", DoubleBass },
		{ "DoubleRhythm]", DoubleRhythm },
		{ "Drums]", Drums },
		{ "Keys]", Keys },
		{ "GHLGuitar]", GHLGuitar },
		{ "GHLBass]", GHLBass },
	};
	for (const auto& track : TRACKS)
		if (validateTrack(track.first))
			return track.second;
	return Invalid;
}

ChtFileReader::NoteTracks_V1 ChtFileReader::extractTrack_V1()
{
	if (!checkDifficulty_V1())
		return Invalid;

	NoteTracks_V1 type = checkTrack_V1();
	if (type != Invalid)
		m_eventSets.push_back(g_validTypes + 2);
	return type;
}

std::vector<Modifiers::Modifier> ChtFileReader::extractModifiers(const ModifierOutline& list)
{
	std::vector<Modifiers::Modifier> modifiers;
	while (isStillCurrentTrack())
	{
		if (auto node = findNode(extractModifierName(), list))
			modifiers.push_back(createModifier(*node));
		nextEvent();
	}
	return modifiers;
}
