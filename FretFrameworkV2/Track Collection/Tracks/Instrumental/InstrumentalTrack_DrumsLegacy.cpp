#include "InstrumentalTrack_DrumsLegacy.h"

bool DifficultyTrack<DrumNote_Legacy>::isOccupied() const { return !m_notes.isEmpty() || !m_events.isEmpty() || !m_specialPhrases.isEmpty(); }

DrumType_Enum DifficultyTrack<DrumNote_Legacy>::load_V1(TxtFileReader& reader)
{
	uint32_t solo = 0;
	DrumType_Enum drumType = DrumType_Enum::LEGACY;

	m_notes.reserve(5000);
	reader.nextEvent();
	while (reader.isStillCurrentTrack())
	{
		const uint32_t position = reader.parsePosition();
		switch (reader.parseEvent())
		{
		case ChartEvent::NOTE:
		{
			auto& note = m_notes.get_or_emplace_back(position);
			auto values = reader.extractColorAndSustain_V1();
			if (!note.set_V1(values.first, values.second))
				throw std::runtime_error("Invalid color value");

			if (drumType == DrumType_Enum::LEGACY)
				drumType = DrumNote_Legacy::testDrumType_V1(values.first);
			break;
		}
		case ChartEvent::SPECIAL:
		{
			auto phrase = reader.extractSpecialPhrase();
			switch (phrase.getType())
			{
			case SpecialPhraseType::StarPower:
			case SpecialPhraseType::StarPowerActivation:
			case SpecialPhraseType::Tremolo:
			case SpecialPhraseType::Trill:
				m_specialPhrases.get_or_emplace_back(position).push_back(phrase);
			}
			break;
		}
		case ChartEvent::EVENT:
		{
			std::string_view str = reader.extractText();
			if (str.starts_with("soloend"))
				m_specialPhrases[position].push_back({ SpecialPhraseType::Solo, position - solo });
			else if (str.starts_with("solo"))
				solo = position;
			else
				m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
			break;
		}
		}
	}

	shrink();
	return drumType;
}

bool InstrumentalTrack<DrumNote_Legacy>::isOccupied() const
{
	for (const auto& diff : m_difficulties)
		if (diff.isOccupied())
			return true;
	return false;
}

DrumType_Enum InstrumentalTrack<DrumNote_Legacy>::getDrumType() const noexcept { return m_drumType; }

void InstrumentalTrack<DrumNote_Legacy>::load_V1(size_t diff, TxtFileReader& reader)
{
	m_drumType = m_difficulties[diff].load_V1(reader);
}

template <>
template <>
DifficultyTrack<DrumNote<4, true>>& DifficultyTrack<DrumNote<4, true>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff)
{
	m_events = std::move(diff.m_events);
	m_specialPhrases = std::move(diff.m_specialPhrases);
	m_notes.reserve(diff.m_notes.size());
	for (const auto& node : diff.m_notes)
		m_notes.emplace_back(node.key) = node->transformNote<4, true>();
	return *this;
}

template <>
template <>
DifficultyTrack<DrumNote<5, false>>& DifficultyTrack<DrumNote<5, false>>::operator=(DifficultyTrack<DrumNote_Legacy>&& diff)
{
	m_events = std::move(diff.m_events);
	m_specialPhrases = std::move(diff.m_specialPhrases);
	m_notes.reserve(diff.m_notes.size());
	for (const auto& node : diff.m_notes)
		m_notes.emplace_back(node.key) = node->transformNote<5, false>();
	return *this;
}
