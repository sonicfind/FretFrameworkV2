#pragma once
#include "VocalTrack.h"
#include "Serialization/MidiFileWriter.h"

namespace Midi_Saver_Vocals
{
	struct MidiWriteNode_Vocals
	{
		std::vector<unsigned char> phrases;
		std::vector<MidiFileWriter::NoteNode> notes;
	};

	template <size_t INDEX, size_t numTracks>
	void Save(const VocalTrack<numTracks>& track, MidiFileWriter& writer)
	{
		SimpleFlatMap<std::vector<std::u32string>> events;
		if constexpr (INDEX == 0)
			events = track.m_events;

		MidiFileWriter::PhraseList phrases;
		if constexpr (INDEX < 2)
		{
			for (const auto& vec : track.m_specialPhrases)
			{
				auto& phraseVec = phrases.emplace_back(vec.key);
				for (const auto& phrase : *vec)
				{
					const uint64_t endPoint = vec.key + phrase.getLength();
					if constexpr (INDEX == 0)
					{
						switch (phrase.type)
						{
						case SpecialPhraseType::StarPower:
							phraseVec.push_back({ 116, endPoint });
							break;
						case SpecialPhraseType::LyricLine:
							phraseVec.push_back({ 105, endPoint });
							break;
						case SpecialPhraseType::RangeShift:
							phraseVec.push_back({ 0, endPoint });
							break;
						case SpecialPhraseType::LyricShift:
							phraseVec.push_back({ 1, endPoint });
							break;
						}
					}
					else if (phrase.type == SpecialPhraseType::HarmonyLine)
						phraseVec.push_back({ 106, endPoint });
				}
			}
		}

		SimpleFlatMap<MidiWriteNode_Vocals> nodes;
		size_t nodeIndex = 0;
		const SimpleFlatMap<Vocal>& vocals = track[INDEX];

		auto eventIter = events.begin();
		auto phraseIter = phrases.begin();
		auto vocalIter = vocals.begin();
		auto percIter = track.m_percussion.begin();
		bool eventValid = eventIter != events.end();
		bool phraseValid = phraseIter != phrases.end();
		bool vocalValid = vocalIter != vocals.end();
		bool percValid = INDEX == 0 ? percIter != track.m_percussion.end() : false;

		while (nodeIndex < nodes.size() || eventValid || phraseValid || vocalValid || percValid)
		{
			while (nodeIndex < nodes.size())
			{
				const auto& node = nodes.at_index(nodeIndex);
				if ((eventValid && eventIter->key < node.key) || (phraseValid && phraseIter->key < node.key) || (vocalValid && vocalIter->key < node.key) || (percValid && percIter->key < node.key))
					break;

				const MidiWriteNode_Vocals& obj = *node;
				for (size_t i = obj.notes.size(); i > 0;)
					writer.writeMidiNote(node.key, obj.notes[--i]);

				for (size_t i = obj.phrases.size(); i > 0;)
					writer.writeMidiNote(node.key, { 0, obj.phrases[--i], 0 });
				++nodeIndex;
			}

			while (eventValid &&
				(nodeIndex >= nodes.size() || eventIter->key < nodes.at_index(nodeIndex).key) &&
				(!phraseValid || eventIter->key <= phraseIter->key) &&
				(!vocalValid || eventIter->key <= vocalIter->key) &&
				(!percValid || eventIter->key <= percIter->key))
			{
				for (const auto& str : eventIter->object)
					writer.writeText(eventIter->key, UnicodeString::U32ToStr(str));
				eventValid = ++eventIter != events.end();
			}

			while (phraseValid &&
				(nodeIndex >= nodes.size() || phraseIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || phraseIter->key < eventIter->key) &&
				(!vocalValid || phraseIter->key <= vocalIter->key) &&
				(!percValid || phraseIter->key <= percIter->key))
			{
				for (const auto& phrase : phraseIter->object)
				{
					writer.writeMidiNote(phraseIter->key, { 0, phrase.first, 100 });
					nodes.find_or_emplace_object(nodeIndex, phrase.second).phrases.push_back(phrase.first);
				}
				phraseValid = ++phraseIter != phrases.end();
			}

			while (vocalValid &&
				(nodeIndex >= nodes.size() || vocalIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || vocalIter->key < eventIter->key) &&
				(!phraseValid || vocalIter->key < phraseIter->key) &&
				(!percValid || vocalIter->key <= percIter->key))
			{
				const Vocal& vocal = vocalIter->object;
				writer.writeText(vocalIter->key, UnicodeString::U32ToStr(vocal.lyric));
				if (vocal.isPlayable())
				{
					const unsigned char binary = vocal.pitch.getBinaryValue();
					writer.writeMidiNote(vocalIter->key, { 0, binary, 100 });
					nodes.find_or_emplace_object(nodeIndex, vocalIter->key + vocal.pitch.getLength()).notes.push_back({ 0, binary, 0 });
				}
				vocalValid = ++vocalIter != vocals.end();
			}

			while (percValid &&
				(nodeIndex >= nodes.size() || percIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || percIter->key < eventIter->key) &&
				(!phraseValid || percIter->key < phraseIter->key) &&
				(!vocalValid || percIter->key < vocalIter->key))
			{
				const VocalPercussion& perc = percIter->object;
				unsigned char note = perc.isPlayable() ? 96 : 97;
				writer.writeMidiNote(percIter->key, { 0, note, 100 });
				nodes.find_or_emplace_object(nodeIndex, percIter->key + 1).notes.push_back({ 0, note, 0 });
				percValid = ++percIter != track.m_percussion.end();
			}
		}

		//for (const auto& note : track[INDEX])
		//{
		//	auto& noteNode = notes.emplace_back(note.key);
		//	writer.addText(nodeIndex, UnicodeString::U32ToStr(note->getLyric()));
		//	if (note->isPlayable())
		//	{
		//		const VocalPitch& pitch = note->getPitch();
		//		writer.addMidiNote(nodeIndex, note.key + note->getLength(), pitch.getBinaryValue(), 100);
		//	}
		//}

		//if constexpr (INDEX == 0)
		//	for (const auto& note : track.m_percussion)
		//		writer.addMidiNote(writer.getNodeIndex(note.key), note.key + 1, note->isPlayable() ? 96 : 97, 100);
	}
}
