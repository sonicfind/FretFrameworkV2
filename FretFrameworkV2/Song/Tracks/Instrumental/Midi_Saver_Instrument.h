#pragma once
#include "InstrumentalTrack.h"
#include "Serialization/MidiFileWriter.h"

namespace Difficulty_Saver_Midi
{
	constexpr char s_OFFSETS[6] = { 60, 72, 84, 96, 120 };

	

	template <class T>
	bool Get_Phrases(const DifficultyTrack<T>& track, MidiFileWriter::PhraseList& phrases, const unsigned char diff)
	{
		const unsigned char OFFSET = s_OFFSETS[diff];

		bool doNextDifficulty = true;
		size_t index = 0;
		for (const auto& vec : track.m_specialPhrases)
		{
			index = phrases.find_or_emplace(index, vec.key);
			auto& vecAdd = *phrases.at_index(index);
			for (const auto& phrase : *vec)
			{
				const uint64_t endPoint = vec.key + phrase.getLength();
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
					vecAdd.push_back({ 116, endPoint });
					doNextDifficulty = false;
					break;
				case SpecialPhraseType::StarPowerActivation:
					vecAdd.push_back({ 120, endPoint });
					vecAdd.push_back({ 121, endPoint });
					vecAdd.push_back({ 122, endPoint });
					vecAdd.push_back({ 123, endPoint });
					vecAdd.push_back({ 124, endPoint });
					doNextDifficulty = false;
					break;
				case SpecialPhraseType::Solo:
					vecAdd.push_back({ 103, endPoint });
					doNextDifficulty = false;
					break;
				case SpecialPhraseType::Tremolo:
					vecAdd.push_back({ 126, endPoint });
					doNextDifficulty = false;
					break;
				case SpecialPhraseType::Trill:
					vecAdd.push_back({ 127, endPoint });
					doNextDifficulty = false;
					break;
				case SpecialPhraseType::StarPower_Diff:
					vecAdd.push_back({ (unsigned char)(OFFSET + 7), endPoint });
					break;
				case SpecialPhraseType::FaceOff_Player1:
					vecAdd.push_back({ (unsigned char)(OFFSET + 9), endPoint });
					break;
				case SpecialPhraseType::FaceOff_Player2:
					vecAdd.push_back({ (unsigned char)(OFFSET + 10), endPoint });
					break;
				}
			}
		}
		return doNextDifficulty;
	}

	template <class T>
	bool Get_Details(const DifficultyTrack<T>& track, MidiFileWriter::SysexList& sysexs, MidiFileWriter::MidiNoteList& notes, const unsigned char diff) { return false; }

	template <class T>
	void Get_Notes(const DifficultyTrack<T>& track, MidiFileWriter::MidiNoteList& notes, const unsigned char diff)
	{
		const unsigned char OFFSET = s_OFFSETS[diff];
		size_t index = 0;
		for (const auto& note : track.m_notes)
		{
			index = notes.find_or_emplace(index, note.key);
			auto& addVec = *notes.at_index(index);
			for (const auto& col : note->getMidiNotes())
				addVec.push_back({ { 0, (unsigned char)(OFFSET + std::get<0>(col)), std::get<1>(col) }, note.key + std::get<2>(col) });
		}
	}
};

namespace Midi_Saver_Instrument
{
	struct MidiWriteNode
	{
		std::vector<MidiFileWriter::NoteNode> notes;
		std::vector<MidiFileWriter::Sysex> sysexs;
		std::vector<unsigned char> phrases;
	};

	template <class T>
	SimpleFlatMap<std::vector<std::u32string>> GetStartingEvent() noexcept { return {}; }

	template <class T>
	void Save(const InstrumentalTrack<T>& track, MidiFileWriter& writer)
	{
		SimpleFlatMap<std::vector<std::u32string>> events = GetStartingEvent<T>();

		for (const auto& vec : track.m_events)
		{
			if (vec.key == 0 && !events.isEmpty())
				events.front().append_range(vec.object);
			else
				events.get_or_emplace_back(vec.key) = vec.object;
		}

		MidiFileWriter::PhraseList phrases;
		for (const auto& vec : track.m_specialPhrases)
		{
			auto& vecAdd = phrases.emplace_back(vec.key);
			for (const auto& phrase : *vec)
			{
				const uint64_t endPoint = vec.key + phrase.getLength();
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
					vecAdd.push_back({ 116, endPoint });
					break;
				case SpecialPhraseType::StarPowerActivation:
					vecAdd.push_back({ 120, endPoint });
					vecAdd.push_back({ 121, endPoint });
					vecAdd.push_back({ 122, endPoint });
					vecAdd.push_back({ 123, endPoint });
					vecAdd.push_back({ 124, endPoint });
					break;
				case SpecialPhraseType::Solo:
					vecAdd.push_back({ 103, endPoint });
					break;
				case SpecialPhraseType::Tremolo:
					vecAdd.push_back({ 126, endPoint });
					break;
				case SpecialPhraseType::Trill:
					vecAdd.push_back({ 127, endPoint });
					break;
				}
			}
		}

		bool doDiff = phrases.isEmpty();
		for (unsigned char i = 0; i < 4 && doDiff; ++i)
			doDiff = Difficulty_Saver_Midi::Get_Phrases(track[i], phrases, i);
		doDiff = true;

		MidiFileWriter::SysexList sysexs;
		MidiFileWriter::MidiNoteList notes;
		for (unsigned char i = 0; i < 4 && doDiff; ++i)
			doDiff = Difficulty_Saver_Midi::Get_Details(track[i], sysexs, notes, i);

		for (unsigned char i = 0; i < 5; ++i)
			Difficulty_Saver_Midi::Get_Notes(track[i], notes, i);

		SimpleFlatMap<MidiWriteNode> nodes;
		size_t nodeIndex = 0;
		auto eventIter = events.begin();
		auto phraseIter = phrases.begin();
		auto sysexIter = sysexs.begin();
		auto noteIter = notes.begin();
		bool eventValid = eventIter != events.end();
		bool phraseValid = phraseIter != phrases.end();
		bool sysexValid = sysexIter != sysexs.end();
		bool noteValid = noteIter != notes.end();

		while (nodeIndex < nodes.size() || eventValid || phraseValid || sysexValid || noteValid)
		{
			while (nodeIndex < nodes.size())
			{
				const auto& node = nodes.at_index(nodeIndex);
				if ((eventValid && eventIter->key < node.key) || (phraseValid && phraseIter->key < node.key) || (sysexValid && sysexIter->key < node.key) || (noteValid && noteIter->key < node.key))
					break;

				const MidiWriteNode& obj = *node;
				for (size_t i = obj.notes.size(); i > 0;)
					writer.writeMidiNote(node.key, obj.notes[--i]);

				for (size_t i = obj.sysexs.size(); i > 0;)
					writer.writeSysex(node.key, obj.sysexs[--i], false);

				for (size_t i = obj.phrases.size(); i > 0;)
					writer.writeMidiNote(node.key, { 0, obj.phrases[--i], 0 });
				++nodeIndex;
			}

			while (eventValid &&
				(nodeIndex >= nodes.size() || eventIter->key < nodes.at_index(nodeIndex).key) &&
				(!phraseValid || eventIter->key <= phraseIter->key) &&
				(!sysexValid || eventIter->key <= sysexIter->key) &&
				(!noteValid || eventIter->key <= noteIter->key))
			{
				for (const auto& str : eventIter->object)
					writer.writeText(eventIter->key, UnicodeString::U32ToStr(str));
				eventValid = ++eventIter != events.end();
			}

			while (phraseValid &&
				(nodeIndex >= nodes.size() || phraseIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || phraseIter->key < eventIter->key) &&
				(!sysexValid || phraseIter->key <= sysexIter->key) &&
				(!noteValid || phraseIter->key <= noteIter->key))
			{
				for (const auto& phrase : phraseIter->object)
				{
					writer.writeMidiNote(phraseIter->key, { 0, phrase.first, 100 });
					nodes.find_or_emplace_object(nodeIndex, phrase.second).phrases.push_back(phrase.first);
				}
				phraseValid = ++phraseIter != phrases.end();
			}

			while (sysexValid &&
				(nodeIndex >= nodes.size() || sysexIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || sysexIter->key < eventIter->key) &&
				(!phraseValid || sysexIter->key < phraseIter->key) &&
				(!noteValid || sysexIter->key <= noteIter->key))
			{
				for (const auto& sysex : sysexIter->object)
				{
					writer.writeSysex(sysexIter->key, sysex.first, true);
					nodes.find_or_emplace_object(nodeIndex, sysex.second).sysexs.push_back(sysex.first);
				}
				sysexValid = ++sysexIter != sysexs.end();
			}

			while (noteValid &&
				(nodeIndex >= nodes.size() || noteIter->key < nodes.at_index(nodeIndex).key) &&
				(!eventValid || noteIter->key < eventIter->key) &&
				(!phraseValid || noteIter->key < phraseIter->key) &&
				(!sysexValid || noteIter->key < sysexIter->key))
			{
				for (auto& note : noteIter->object)
				{
					writer.writeMidiNote(noteIter->key, note.first);
					note.first.velocity = 0;
					nodes.find_or_emplace_object(nodeIndex, note.second).notes.push_back(note.first);
				}
				noteValid = ++noteIter != notes.end();
			}
		}
	}
}
