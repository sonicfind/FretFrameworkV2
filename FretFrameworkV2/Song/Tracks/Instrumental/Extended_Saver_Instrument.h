#pragma once
#include "InstrumentalTrack.h"
#include "Song/Tracks/Extended_Save.h"

template <class T>
class Extended_Save::WriteNode<DifficultyTrack<T>> : public Extended_Save::WriteNode<Track>
{
public:
	void write(const uint32_t position, CommonChartWriter& writer) const noexcept
	{
		writePhrases(position, writer);
		writeNote(position, writer);
		writeEvents(position, writer);
	}

	using Extended_Save::WriteNode<Track>::set;
	void set(const T& note) { m_note = &note; }

	static SimpleFlatMap<WriteNode> GetNodes(const DifficultyTrack<T>& track)
	{
		SimpleFlatMap<WriteNode> nodes(track.m_specialPhrases.size() + track.m_events.size() + track.m_notes.size());
		for (const auto& note : track.m_specialPhrases)
			nodes.emplace_back(note.key).set(*note);

		for (const auto& note : track.m_notes)
			nodes[note.key].set(*note);

		for (const auto& events : track.m_events)
			nodes[events.key].set(*events);
		return nodes;
	}

protected:
	void writeNote(const uint32_t position, CommonChartWriter& writer)  const noexcept
	{
		if (m_note == nullptr)
			return;

		std::vector<std::pair<size_t, uint32_t>> colors = m_note->getActiveColors();
		if (colors.size() == 1)
		{
			writer.startEvent(position, ChartEvent::NOTE);
			writer.writeSingleNote(colors[0]);
			writer.writeSingleNoteMods(m_note->getActiveModifiers(colors[0].first));
			writer.finishEvent();
		}
		else if (colors.size() > 1)
		{
			writer.startEvent(position, ChartEvent::MULTI_NOTE);
			writer.writeMultiNote(colors);
			writer.finishEvent();

			const std::vector<std::pair<char, size_t>> modifiers = m_note->getActiveModifiers();
			if (!modifiers.empty())
			{
				writer.startEvent(position, ChartEvent::MODIFIER);
				writer.writeMultiNoteMods(modifiers);
				writer.finishEvent();
			}
		}
	}

private:
	const T* m_note = nullptr;
};

namespace Instrument_Save
{
	template <class T>
	void Save(const InstrumentalTrack<T>& track, CommonChartWriter& writer)
	{
		Extended_Save::Save(track, writer);
		for (unsigned char i = 0; i < 5; ++i)
		{
			const DifficultyTrack<T>& diff = track[i];
			if (diff.isOccupied())
			{
				writer.writeDifficultyTrack(i);
				Extended_Save::Save(diff, writer);
				writer.finishTrack();
			}
		}
	}
}
