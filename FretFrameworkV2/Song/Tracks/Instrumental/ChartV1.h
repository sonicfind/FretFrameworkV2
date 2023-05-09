#pragma once
#include "InstrumentalTrack.h"

namespace ChartV1
{
	template <class T>
	class V1Loader
	{
	public:
		[[nodiscard]] bool addNote(SimpleFlatMap<T>& notes, uint32_t position, std::pair<size_t, uint32_t> note)
		{
			return notes.get_or_emplace_back(position).set_V1(note.first, note.second);
		}
	};

	template <class T>
	void Load(InstrumentalTrack<T>& track, ChtFileReader& reader)
	{
		V1Loader<T> loader;
		Load(loader, track, reader);
	}

	template <class T>
	bool Load(V1Loader<T>& loader, InstrumentalTrack<T>& track, ChtFileReader& reader)
	{
		DifficultyTrack<T>& diff = track[reader.getDifficulty()];
		if (diff.isOccupied())
			return false;

		uint32_t solo = 0;
		diff.m_notes.reserve(5000);
		while (reader.isStillCurrentTrack())
		{
			const auto trackEvent = reader.parseEvent();
			switch (trackEvent.second)
			{
			case ChartEvent::NOTE:
			{
				if (!loader.addNote(diff.m_notes, trackEvent.first, reader.extractColorAndSustain_V1()))
					if (!diff.m_notes.back().validate())
						diff.m_notes.pop_back();
				break;
			}
			case ChartEvent::SPECIAL:
			{
				auto phrase = reader.extractSpecialPhrase();
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
				case SpecialPhraseType::StarPowerActivation:
				case SpecialPhraseType::Tremolo:
				case SpecialPhraseType::Trill:
					diff.m_specialPhrases.get_or_emplace_back(trackEvent.first).push_back(phrase);
				}
				break;
			}
			case ChartEvent::EVENT:
			{
				std::string_view str = reader.extractText();
				if (str.starts_with("soloend"))
					diff.m_specialPhrases[trackEvent.first].push_back({ SpecialPhraseType::Solo, trackEvent.first - solo });
				else if (str.starts_with("solo"))
					solo = trackEvent.first;
				else
					diff.m_events.get_or_emplace_back(trackEvent.first).push_back(UnicodeString::strToU32(str));
				break;
			}
			}
			reader.nextEvent();
		}
		diff.shrink();
		return true;
	}
};
