#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
namespace Midi_Loader
{
	template <class PhraseType>
	struct PhraseNode
	{
		PhraseType type;
		uint64_t position = UINT64_MAX;
		unsigned char velocity = 0;
		PhraseNode(PhraseType typ) : type(typ) {}
	};

	template <class T>
	auto& GetNode(SimpleFlatMap<T>& map, uint64_t position)
	{
		auto iter = map.end();
		while (iter != map.begin())
		{
			--iter;
			if (iter->key <= position)
				break;
		}
		return *iter;
	}

	template <class PhraseType>
	class Loader_Phrases
	{
		std::vector<std::pair<std::vector<unsigned char>, Midi_Loader::PhraseNode<PhraseType>>> m_phrases;

	public:
		Loader_Phrases(const std::vector<std::pair<std::vector<unsigned char>, Midi_Loader::PhraseNode<PhraseType>>>& phrases) : m_phrases(phrases) {}

		template <bool NoteOn, class Phrase>
		bool addPhrase(SimpleFlatMap<std::vector<Phrase>>& phrases, const uint64_t position, const MidiNote note) noexcept
		{
			for (auto& phrase : m_phrases)
			{
				for (unsigned char val : phrase.first)
				{
					if (val == note.value)
					{
						auto& phr = phrase.second;
						if constexpr (NoteOn)
						{
							phrases.get_or_emplace_back(position);
							phr.position = position;
							phr.velocity = note.velocity;
						}
						else if (phr.position != UINT64_MAX)
						{
							Midi_Loader::GetNode(phrases, phr.position)->push_back({ phr.type, position - phr.position, phr.velocity });
							phr.position = UINT64_MAX;
						}
						return true;
					}
				}
			}
			return false;
		}

		template <bool NoteOn, class Phrase>
		void addPhrase(SimpleFlatMap<std::vector<Phrase>>& phrases, const uint64_t position, const PhraseType type, const unsigned char velocity)
		{
			for (auto& phrase : m_phrases)
			{
				auto& phr = phrase.second;
				if (phr.type == type)
				{
					if constexpr (NoteOn)
					{
						phrases.get_or_emplace_back(position);
						phr.position = position;
						phr.velocity = velocity;
					}
					else if (phr.position != UINT64_MAX)
					{
						Midi_Loader::GetNode(phrases, phr.position)->push_back({ phr.type, position - phr.position, phr.velocity });
						phr.position = UINT64_MAX;
					}
					return;
				}
			}
			throw std::runtime_error("Invalid phrase type chosen");
		}
	};
}
