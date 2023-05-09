#pragma once
#include "InstrumentalTrack.h"
#include "Serialization/MidiFileWriter.h"

template <class T>
class Difficulty_Saver_Midi
{
public:
	Difficulty_Saver_Midi(bool phrases) : m_phrases(phrases) {}

	void save_events_phrases(const DifficultyTrack<T>& track, const unsigned char index, MidiFileWriter& writer)
	{
		const char OFFSET = s_OFFSETS[index];
		if (!m_phrases)
			goto Details;

		for (const auto& vec : track.m_specialPhrases)
		{
			for (const auto& phrase : *vec)
			{
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
					writer.addMidiNote(vec.key, 116, 100, phrase.getLength());
					m_phrases = false;
					break;
				case SpecialPhraseType::StarPowerActivation:
					writer.addMidiNote(vec.key, 120, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 121, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 122, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 123, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 124, 100, phrase.getLength());
					m_phrases = false;
					break;
				case SpecialPhraseType::Solo:
					writer.addMidiNote(vec.key, 103, 100, phrase.getLength());
					m_phrases = false;
					break;
				case SpecialPhraseType::Tremolo:
					writer.addMidiNote(vec.key, 126, 100, phrase.getLength());
					m_phrases = false;
					break;
				case SpecialPhraseType::Trill:
					writer.addMidiNote(vec.key, 127, 100, phrase.getLength());
					m_phrases = false;
					break;
				case SpecialPhraseType::StarPower_Diff:
					writer.addMidiNote(vec.key, OFFSET + 7, 100, phrase.getLength());
					break;
				case SpecialPhraseType::FaceOff_Player1:
					writer.addMidiNote(vec.key, OFFSET + 9, 100, phrase.getLength());
					break;
				case SpecialPhraseType::FaceOff_Player2:
					writer.addMidiNote(vec.key, OFFSET + 10, 100, phrase.getLength());
					break;
				}
			}
		}

	Details:
		write_details(track, index, writer);
	}

	void save_notes(const DifficultyTrack<T>& track, const unsigned char index, MidiFileWriter& writer) const
	{
		const char OFFSET = s_OFFSETS[index];
		for (const auto& note : track.m_notes)
			for (const auto& col : note->getMidiNotes())
				writer.addMidiNote(note.key, OFFSET + std::get<0>(col), std::get<1>(col), std::get<2>(col));
	}

private:
	void write_details(const DifficultyTrack<T>& track, const unsigned char index, MidiFileWriter& writer) {}

private:
	static constexpr char s_OFFSETS[6] = { 60, 72, 84, 96, 120 };

	bool m_details = true;
	bool m_phrases;
};

namespace Midi_Saver_Instrument
{
	template <class T>
	void WriteMidiToggleEvent(MidiFileWriter& writer) {}

	template <class T>
	void Save(const InstrumentalTrack<T>& track, MidiFileWriter& writer)
	{
		WriteMidiToggleEvent<T>(writer);

		for (const auto& vec : track.m_events)
			for (const auto& ev : *vec)
				writer.addText(vec.key, UnicodeString::U32ToStr(ev));

		Difficulty_Saver_Midi<T> diffSaver(track.m_specialPhrases.isEmpty());
		for (const auto& vec : track.m_specialPhrases)
		{
			for (const auto& phrase : *vec)
			{
				switch (phrase.type)
				{
				case SpecialPhraseType::StarPower:
					writer.addMidiNote(vec.key, 116, 100, phrase.getLength());
					break;
				case SpecialPhraseType::StarPowerActivation:
					writer.addMidiNote(vec.key, 120, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 121, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 122, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 123, 100, phrase.getLength());
					writer.addMidiNote(vec.key, 124, 100, phrase.getLength());
					break;
				case SpecialPhraseType::Solo:
					writer.addMidiNote(vec.key, 103, 100, phrase.getLength());
					break;
				case SpecialPhraseType::Tremolo:
					writer.addMidiNote(vec.key, 126, 100, phrase.getLength());
					break;
				case SpecialPhraseType::Trill:
					writer.addMidiNote(vec.key, 127, 100, phrase.getLength());
					break;
				}
			}
		}

		for (unsigned char i = 0; i < 4; ++i)
			diffSaver.save_events_phrases(track[i], i, writer);

		for (unsigned char i = 0; i < 5; ++i)
			diffSaver.save_notes(track[i], i, writer);
	}
}
