#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
namespace Midi_Loader
{
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

	template <bool NoteOn, class Phrase, class PhraseType>
	void AddPhrase(SimpleFlatMap<std::vector<Phrase>>& phrases, std::pair<PhraseType, uint64_t>& combo, const uint64_t position)
	{
		if constexpr (NoteOn)
		{
			phrases.get_or_emplace_back(position);
			combo.second = position;
		}
		else if (combo.second != UINT64_MAX)
		{
			Midi_Loader::GetNode(phrases, combo.second)->push_back({ combo.first, position - combo.second });
			combo.second = UINT64_MAX;
		}
	}
}
