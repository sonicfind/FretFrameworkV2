#pragma once
#include "VocalTrack.h"
#include "Song/Tracks/Extended_Save.h"

template <size_t numTracks>
class Extended_Save::WriteNode<VocalTrack<numTracks>> : public Extended_Save::WriteNode<Track>
{
public:
	void write(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		writePhrases(position, writer);
		writeVocals(position, writer);
		writePercussion(position, writer);
		writeEvents(position, writer);
	}

	using Extended_Save::WriteNode<Track>::set;
	void set(size_t index, const Vocal& note) { m_vocals[index] = &note; }
	void set(const VocalPercussion& note) { m_perc = &note; }

	static SimpleFlatMap<WriteNode> GetNodes(const VocalTrack<numTracks>& track)
	{
		size_t reserveSpace = track.m_specialPhrases.size() + track.m_events.size();
		for (size_t i = 0; i < numTracks; ++i)
			reserveSpace += track.m_vocals[i].size();

		SimpleFlatMap<WriteNode> nodes(reserveSpace);
		for (const auto& phrases : track.m_specialPhrases)
			nodes.emplace_back(phrases.key).set(*phrases);

		for (size_t i = 0; i < numTracks; ++i)
			for (const auto& vocal : track[i])
				nodes[vocal.key].set(i, *vocal);

		for (const auto& perc : track.m_percussion)
			nodes[perc.key].set(*perc);

		for (const auto& events : track.m_events)
			nodes[events.key].set(*events);
		return nodes;
	}

protected:
	void writeVocals(const uint64_t position, CommonChartWriter& writer) const
	{
		for (size_t i = 0; i < numTracks; ++i)
		{
			if (m_vocals[i] == nullptr)
				continue;

			const Vocal* const vocal = m_vocals[i];
			if (vocal->isPlayable())
			{
				writer.startEvent(position, ChartEvent::VOCAL);
				writer.writeLyric({ i + 1, UnicodeString::U32ToStr(vocal->getLyric()) });
				writer.writePitchAndDuration(vocal->getPitchAndDuration());
			}
			else
			{
				writer.startEvent(position, ChartEvent::LYRIC);
				writer.writeLyric({ i + 1, UnicodeString::U32ToStr(vocal->getLyric()) });
			}
			writer.finishEvent();
		}
	}

	void writePercussion(const uint64_t position, CommonChartWriter& writer) const
	{
		if (m_perc == nullptr)
			return;

		writer.startEvent(position, ChartEvent::VOCAL_PERCUSSION);
		writer.writeSingleNoteMods(m_perc->getActiveModifiers());
		writer.finishEvent();
	}

private:
	const Vocal* m_vocals[numTracks] = {};
	const VocalPercussion* m_perc = nullptr;
};
