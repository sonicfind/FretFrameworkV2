#include "Song.h"
#include "Tracks/Instrumental/Extended_Saver_Instrument.h"
#include "Tracks/Vocal/Extended_Saver_Vocals.h"
#include "Serialization/BCHFileWriter.h"
#include "Serialization/ChtFileWriter.h"

template<>
class Extended_Save::WriteNode<SyncTrack>;

template<>
class Extended_Save::WriteNode<SongEvents>;

template <class T>
void Save(const InstrumentalTrack<T>& track, CommonChartWriter& writer)
{
	Instrument_Save::Save(track, writer);
}

template <size_t numTracks>
void Save(const VocalTrack<numTracks>& track, CommonChartWriter& writer)
{
	Extended_Save::Save(track, writer);
}

void Song::save_extended(const std::filesystem::path& path, bool binary) const
{
	std::unique_ptr<CommonChartWriter> writer;
	if (binary)
		writer.reset(new BCHFileWriter(path));
	else
		writer.reset(new ChtFileWriter(path));

	writer->writeHeaderTrack(m_tickrate);
	writer->finishTrack();

	writer->writeSyncTrack();
	Extended_Save::Save(m_sync, *writer);
	writer->finishTrack();
	
	writer->writeEventTrack();
	Extended_Save::Save(m_events, *writer);
	writer->finishTrack();

	const auto saveTrack = [&writer](const auto& track, unsigned char index)
	{
		if (track.isOccupied())
		{
			writer->writeNoteTrack(index);
			Save(track, *writer);
			writer->finishTrack();
		}
	};

	saveTrack(m_noteTracks.lead_5, 0);
	saveTrack(m_noteTracks.lead_6, 1);
	saveTrack(m_noteTracks.bass_5, 2);
	saveTrack(m_noteTracks.bass_6, 3);
	saveTrack(m_noteTracks.rhythm, 4);
	saveTrack(m_noteTracks.coop, 5);
	saveTrack(m_noteTracks.keys, 6);
	saveTrack(m_noteTracks.drums4_pro, 7);
	saveTrack(m_noteTracks.drums5, 8);
	saveTrack(m_noteTracks.vocals, 9);
	saveTrack(m_noteTracks.harmonies, 10);
}

template<>
class Extended_Save::WriteNode<SyncTrack>
{
public:
	void write(const uint64_t position, CommonChartWriter& writer)  const noexcept
	{
		writeMicros(position, writer);
		writeAnchor(position, writer);
		writeTimeSig(position, writer);
	}

	void set(const MicrosPerQuarter& micros) { m_micros = &micros; }
	void set(const uint64_t& anchor) { m_anchor = &anchor; }
	void set(const TimeSig& timeSig) { m_timeSig = &timeSig; }

	static SimpleFlatMap<WriteNode> GetNodes(const SyncTrack& track)
	{
		SimpleFlatMap<WriteNode> nodes(track.timeSigs.size() + track.tempoMarkers.size());
		for (const auto& timeSig : track.timeSigs)
			nodes.emplace_back(timeSig.key).set(*timeSig);

		bool isFirst = true;
		for (const auto& micros : track.tempoMarkers)
		{
			auto& node = nodes[micros.key];
			node.set(micros->first);
			if (isFirst || micros->second > 0)
				node.set(micros->second);
			isFirst = false;
		}
		return nodes;
	}

protected:
	void writeMicros(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		if (m_micros)
		{
			writer.startEvent(position, ChartEvent::BPM);
			writer.writeMicrosPerQuarter(*m_micros);
			writer.finishEvent();
		}
	}

	void writeAnchor(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		if (m_anchor)
		{
			writer.startEvent(position, ChartEvent::ANCHOR);
			writer.writeAnchor(*m_anchor);
			writer.finishEvent();
		}
	}

	void writeTimeSig(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		if (m_timeSig)
		{
			writer.startEvent(position, ChartEvent::TIME_SIG);
			writer.writeTimeSig(*m_timeSig);
			writer.finishEvent();
		}
	}

private:
	const MicrosPerQuarter* m_micros = nullptr;
	const uint64_t* m_anchor = nullptr;
	const TimeSig* m_timeSig = nullptr;
};

template<>
class Extended_Save::WriteNode<SongEvents>
{
public:
	void write(const uint64_t position, CommonChartWriter& writer)  const noexcept
	{
		writeSection(position, writer);
		writeEvents(position, writer);
	}

	void set(const UnicodeString& section) { m_section = &section; }
	void set(const std::vector<std::u32string>& events) { m_events = &events; }

	static SimpleFlatMap<WriteNode> GetNodes(const SongEvents& track)
	{
		SimpleFlatMap<WriteNode> nodes(track.sections.size() + track.globals.size());
		for (const auto& section : track.sections)
			nodes.emplace_back(section.key).set(*section);

		for (const auto& global : track.globals)
			nodes[global.key].set(*global);
		return nodes;
	}

protected:
	void writeSection(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		if (m_section)
		{
			writer.startEvent(position, ChartEvent::SECTION);
			writer.writeText(m_section->toString());
			writer.finishEvent();
		}
	}

	void writeEvents(const uint64_t position, CommonChartWriter& writer) const noexcept
	{
		if (m_events)
		{
			for (const auto& ev : *m_events)
			{
				writer.startEvent(position, ChartEvent::EVENT);
				writer.writeText(UnicodeString::U32ToStr(ev));
				writer.finishEvent();
			}
		}
	}

private:
	const UnicodeString* m_section = nullptr;
	const std::vector<std::u32string>* m_events = nullptr;
};
