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

	template <bool NoteOn, class Phrase, class PhraseType>
	void AddPhrase(SimpleFlatMap<std::vector<Phrase>>& phrases, PhraseNode<PhraseType>& phrase, const uint64_t position, const unsigned char velocity)
	{
		if constexpr (NoteOn)
		{
			phrases.get_or_emplace_back(position);
			phrase.position = position;
			phrase.velocity = velocity;
		}
		else if (phrase.position != UINT64_MAX)
		{
			Midi_Loader::GetNode(phrases, phrase.position)->push_back({ phrase.type, position - phrase.position, phrase.velocity });
			phrase.position = UINT64_MAX;
		}
	}
}
