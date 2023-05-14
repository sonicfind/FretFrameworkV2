#pragma once
#include "../Track.h"
#include "Notes/VocalNote.h"
#include "Notes/VocalPercussion.h"
#include "Serialization/MidiFileReader.h"
#include "Serialization/MidiFileWriter.h"
#include <assert.h>

template <size_t numTracks>
class VocalTrack : public Track
{
public:
	SimpleFlatMap<Vocal> m_vocals[numTracks];
	SimpleFlatMap<VocalPercussion> m_percussion;

public:
	[[nodiscard]] virtual bool isOccupied() const override
	{
		for (const auto& track : m_vocals)
			if (!track.isEmpty())
				return true;

		return !m_percussion.isEmpty() || !m_specialPhrases.isEmpty() || !m_events.isEmpty();
	}

	virtual void clear() override
	{
		for (auto& track : m_vocals)
			track.clear();
		m_percussion.clear();
		m_events.clear();
		m_specialPhrases.clear();
	}

	virtual void adjustTicks(double multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& track : m_vocals)
		{
			for (auto& vocal : track)
			{
				vocal.key = uint64_t(vocal.key * multiplier);
				vocal->pitch *= multiplier;
			}
		}

		for (auto& perc : m_percussion)
			perc.key = uint64_t(perc.key * multiplier);
	}

	void shrink()
	{
		for (auto& track : m_vocals)
			if ((track.size() < 100 || 2000 <= track.size()) && track.size() < track.capacity())
				track.shrink_to_fit();

		if ((m_percussion.size() < 20 || 400 <= m_percussion.size()) && m_percussion.size() < m_percussion.capacity())
			m_percussion.shrink_to_fit();
	}

	[[nodiscard]] bool hasNotes(size_t index) const
	{
		if (index == 0)
			return !m_vocals[index].isEmpty() || !m_percussion.isEmpty();
		else
			return !m_vocals[index].isEmpty();
	}

	[[nodiscard]] bool isOccupied(size_t index) const
	{
		if (hasNotes(index))
			return true;

		if (index == 2)
			return false;

		if (index == 0 && !m_events.isEmpty())
			return true;

		for (const auto& vec : m_specialPhrases)
		{
			for (const auto& phrase : *vec)
			{
				if (index == 0)
				{
					switch (phrase.type)
					{
					case SpecialPhraseType::StarPower:
					case SpecialPhraseType::LyricLine:
					case SpecialPhraseType::RangeShift:
					case SpecialPhraseType::LyricShift:
						return true;
					}
				}
				else if (phrase.type == SpecialPhraseType::HarmonyLine)
					return true;
			}
		}
		return false;
	}

	SimpleFlatMap<Vocal>& operator[](size_t i)
	{
		assert(i < numTracks);
		return m_vocals[i];
	}

	const SimpleFlatMap<Vocal>& operator[](size_t i) const
	{
		assert(i < numTracks);
		return m_vocals[i];
	}
};
