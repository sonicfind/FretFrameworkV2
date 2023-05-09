#pragma once
#include "VocalTrack.h"
#include "Serialization/MidiFileWriter.h"

namespace Midi_Saver_Vocals
{
	template <size_t INDEX, size_t numTracks>
	void Save(const VocalTrack<numTracks>& track, MidiFileWriter& writer)
	{
		if constexpr (INDEX == 0)
		{
			for (const auto& vec : track.m_events)
				for (const auto& ev : *vec)
					writer.addText(vec.key, UnicodeString::U32ToStr(ev));
		}

		if constexpr (INDEX < 2)
		{
			for (const auto& vec : track.m_specialPhrases)
			{
				for (const auto& phrase : *vec)
				{
					if constexpr (INDEX == 0)
					{
						switch (phrase.getType())
						{
						case SpecialPhraseType::StarPower:
							writer.addMidiNote(vec.key, 116, 100, phrase.getDuration());
							break;
						case SpecialPhraseType::LyricLine:
							writer.addMidiNote(vec.key, 105, 100, phrase.getDuration());
							break;
						case SpecialPhraseType::RangeShift:
							writer.addMidiNote(vec.key, 0, 100, phrase.getDuration());
							break;
						case SpecialPhraseType::LyricShift:
							writer.addMidiNote(vec.key, 1, 100, phrase.getDuration());
							break;
						}
					}
					else if (phrase.getType() == SpecialPhraseType::HarmonyLine)
						writer.addMidiNote(vec.key, 106, 100, phrase.getDuration());
				}
			}
		}

		for (const auto& note : track[INDEX])
		{
			writer.addText(note.key, UnicodeString::U32ToStr(note->getLyric()));
			if (note->isPlayable())
			{
				const VocalPitch& pitch = note->getPitch();
				writer.addMidiNote(note.key, pitch.getBinaryValue(), 100, note->getDuration());
			}
		}

		if constexpr (INDEX == 0)
			for (const auto& note : track.m_percussion)
				writer.addMidiNote(note.key, note->isPlayable() ? 96 : 97, 100, 1);
	}
}
