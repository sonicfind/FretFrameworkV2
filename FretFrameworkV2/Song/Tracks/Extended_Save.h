#pragma once
#include "Track.h"
#include "Serialization/CommonChartWriter.h"

namespace Extended_Save
{
	template <class T>
	class WriteNode
	{
	public:
		void write(const uint64_t position, CommonChartWriter& writer)  const noexcept
		{
			writePhrases(position, writer);
			writeEvents(position, writer);
		}

		void set(const std::vector<SpecialPhrase>& phrases) { m_phrases = &phrases; }
		void set(const std::vector<std::u32string>& events) { m_events = &events; }

		static SimpleFlatMap<WriteNode> GetNodes(const T& track)
		{
			SimpleFlatMap<WriteNode> nodes(track.m_specialPhrases.size() + track.m_events.size());
			for (const auto& note : track.m_specialPhrases)
				nodes.emplace_back(note.key).set(*note);

			for (const auto& events : track.m_events)
				nodes[events.key].set(*events);
			return nodes;
		}

	protected:
		void writePhrases(const uint64_t position, CommonChartWriter& writer) const noexcept
		{
			if (m_phrases)
			{
				for (const auto& phrase : *m_phrases)
				{
					writer.startEvent(position, ChartEvent::SPECIAL);
					writer.writeSpecialPhrase(phrase);
					writer.finishEvent();
				}
			}
		}

		void writeEvents(const uint64_t position, CommonChartWriter& writer) const noexcept
		{
			if (m_events)
			{
				for (const auto& str : *m_events)
				{
					writer.startEvent(position, ChartEvent::EVENT);
					writer.writeText(UnicodeString::U32ToStr(str));
					writer.finishEvent();
				}
			}
		}

	private:
		const std::vector<SpecialPhrase>* m_phrases = nullptr;
		const std::vector<std::u32string>* m_events = nullptr;
	};

	template <class T>
	void Save(const T& track, CommonChartWriter& writer)
	{
		for (const auto& node : WriteNode<T>::GetNodes(track))
			node->write(node.key, writer);
	}
}

