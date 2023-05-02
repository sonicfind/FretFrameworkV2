#include "Song.h"
#include <iostream>

void Song::setMetaData(const LibraryEntry& entry)
{
	m_name = entry.getName().get();
	m_artist = entry.getName().get();
	m_album = entry.getName().get();
	m_genre = entry.getName().get();
	m_year = entry.getName().get();
	m_charter = entry.getName().get();
	m_playlist = entry.getName().get();
	m_baseDrumType = entry.getDrumType();
	m_hopo_frequency = entry.getHopoFrequency();
	m_midiStarPowerNote = entry.getMidiStarPowerNote();
}

EntryStatus Song::load(const LibraryEntry& entry)
{
	clear();
	setMetaData(entry);
	const std::filesystem::directory_entry fileEntry = entry.getFileEntry();
	if (!fileEntry.exists() || fileEntry.last_write_time() != entry.getLastWriteTime())
		return EntryStatus::NEEDS_RESCAN;

	if (!loadIni(entry.getDirectory() / U"song.ini"))
		return EntryStatus::NEEDS_RESCAN;

	try
	{
		switch (entry.getChartType())
		{
		case ChartType::BCH:
			load_bch(fileEntry.path());
			break;
		case ChartType::CHT:
			load_cht(fileEntry.path());
			break;
		case ChartType::MID:
			load_mid(fileEntry.path());
			break;
		default:
			break;
		}
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return EntryStatus::ERROR;
	}
	return EntryStatus::UNCHANGED;
}

bool Song::load(const std::pair<std::filesystem::path, ChartType>& chartFile) noexcept
{
	clear();
	setMetaData(chartFile.first.parent_path() / U"song.ini");

	try
	{
		switch (chartFile.second)
		{
		case ChartType::BCH:
			load_bch(chartFile.first);
			break;
		case ChartType::CHT:
			load_cht(chartFile.first);
			break;
		case ChartType::MID:
			load_mid(chartFile.first);
			break;
		default:
			break;
		}
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}
	return true;
}

void Song::resetTempoMap()
{
	m_tempoMarkers.clear();
	m_tempoMarkers.emplace_back(0);

	m_timeSigs.clear();
	m_timeSigs.emplace_back(0) = { 4, 2, 24, 8 };
}

bool Song::save(std::filesystem::path path) noexcept
{
	enum EXT
	{
		BCH,
		CHT,
		MID
	};

	try
	{
		save_cht(path.replace_extension(".cht"));
		save_bch(path.replace_extension(".bch"));
		save_mid(path.replace_extension(".mid"));
	}
	catch (std::runtime_error err)
	{
		std::cout << err.what() << std::endl;
		return false;
	}
	return true;
}

void Song::clear()
{
	resetTempoMap();
	m_sectionMarkers.clear();
	m_globalEvents.clear();
	m_noteTracks.lead_5.clear();
	m_noteTracks.lead_6.clear();
	m_noteTracks.bass_5.clear();
	m_noteTracks.bass_6.clear();
	m_noteTracks.rhythm.clear();
	m_noteTracks.coop.clear();
	m_noteTracks.keys.clear();
	m_noteTracks.drums4_pro.clear();
	m_noteTracks.drums5.clear();
	m_noteTracks.vocals.clear();
	m_noteTracks.harmonies.clear();
}

void Song::traverse(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
			load_noteTrack(parser);
		else if (parser.validateSyncTrack())
			load_tempoMap(parser);
		else if (parser.validateEventTrack())
			load_events(parser);
		else
			parser.skipTrack();
	}
}

void Song::load_tempoMap(CommonChartParser& parser)
{
	while (parser.isStillCurrentTrack())
	{
		const auto trackEvent = parser.parseEvent();
		switch (trackEvent.second)
		{
		case ChartEvent::BPM:
			m_tempoMarkers.get_or_emplace_back(trackEvent.first) = parser.extractMicrosPerQuarter();
			break;
		case ChartEvent::TIME_SIG:
			m_timeSigs.get_or_emplace_back(trackEvent.first).combine(parser.extractTimeSig());
			break;
		}
		parser.nextEvent();
	}
}

void Song::load_events(CommonChartParser& parser)
{
	while (parser.isStillCurrentTrack())
	{
		const auto trackEvent = parser.parseEvent();
		switch (trackEvent.second)
		{
		case ChartEvent::EVENT:
		{
			std::string_view str = parser.extractText();
			if (str.starts_with("section "))
				m_sectionMarkers.get_or_emplace_back(trackEvent.first) = str.substr(8);
			else
				m_globalEvents.get_or_emplace_back(trackEvent.first).push_back(UnicodeString::strToU32(str));
			break;
		}
		case ChartEvent::SECTION:
			m_sectionMarkers.get_or_emplace_back(trackEvent.first) = parser.extractText();
			break;
		}
		parser.nextEvent();
	}
}

void Song::load_noteTrack(CommonChartParser& parser)
{
	BCH_CHT_Extensions* const arr[11] =
	{
		&m_noteTracks.lead_5,
		&m_noteTracks.lead_6,
		&m_noteTracks.bass_5,
		&m_noteTracks.bass_6,
		&m_noteTracks.rhythm,
		&m_noteTracks.coop,
		&m_noteTracks.keys,
		&m_noteTracks.drums4_pro,
		&m_noteTracks.drums5,
		&m_noteTracks.vocals,
		&m_noteTracks.harmonies
	};

	const size_t index = parser.geNoteTrackID();
	if (index < std::size(arr))
		arr[parser.geNoteTrackID()]->load(parser);
	else //BCH only
		parser.skipTrack();
}

void Song::save(CommonChartWriter& writer) const
{
	save_header(writer);
	save_tempoMap(writer);
	save_events(writer);
	save_noteTracks(writer);
}

void Song::save_header(CommonChartWriter& writer) const
{
	writer.writeHeaderTrack(m_tickrate);
	writer.finishTrack();
}

void Song::save_tempoMap(CommonChartWriter& writer) const
{
	writer.writeSyncTrack();
	auto tempo = m_tempoMarkers.begin();
	auto timeSig = m_timeSigs.begin();
	while (tempo != m_tempoMarkers.end() || timeSig != m_timeSigs.end())
	{
		while (tempo != m_tempoMarkers.end() && (timeSig == m_timeSigs.end() || tempo->key <= timeSig->key))
		{
			writer.startEvent(tempo->key, ChartEvent::BPM);
			writer.writeMicrosPerQuarter(**tempo);
			writer.finishEvent();
			++tempo;
		}

		while (timeSig != m_timeSigs.end() && (tempo == m_tempoMarkers.end() || timeSig->key < tempo->key))
		{
			writer.startEvent(timeSig->key, ChartEvent::TIME_SIG);
			writer.writeTimeSig(**timeSig);
			writer.finishEvent();
			++timeSig;
		}
	}
	writer.finishTrack();
}

void Song::save_events(CommonChartWriter& writer) const
{
	struct EventPointer
	{
		const UnicodeString* m_section = nullptr;
		const std::vector<std::u32string>* m_events = nullptr;

		void writeSection(uint32_t position, CommonChartWriter& writer) const
		{
			if (m_section == nullptr)
				return;

			writer.startEvent(position, ChartEvent::SECTION);
			writer.writeText(m_section->toString());
			writer.finishEvent();
		}

		void writeEvents(uint32_t position, CommonChartWriter& writer) const
		{
			if (m_events == nullptr)
				return;

			for (const std::u32string& ev : *m_events)
			{
				writer.startEvent(position, ChartEvent::EVENT);
				writer.writeText(UnicodeString::U32ToStr(ev));
				writer.finishEvent();
			}
		}
	};

	writer.writeEventTrack();

	SimpleFlatMap<EventPointer> nodes;
	for (const auto& section : m_sectionMarkers)
		nodes.emplace_back(section.key).m_section = &section.object;

	for (const auto& events : m_globalEvents)
		nodes[events.key].m_events = &events.object;

	for (const auto& node : nodes)
	{
		node->writeSection(node.key, writer);
		node->writeEvents(node.key, writer);
	}
	
	writer.finishTrack();
}

void Song::save_noteTracks(CommonChartWriter& writer) const
{
	const auto write = [&writer](const auto& track, unsigned char index)
	{
		if (track.isOccupied())
		{
			writer.writeNoteTrack(index);
			track.save(writer);
			writer.finishTrack();
		}
	};
	write(m_noteTracks.lead_5, 0);
	write(m_noteTracks.lead_6, 1);
	write(m_noteTracks.bass_5, 2);
	write(m_noteTracks.bass_6, 3);
	write(m_noteTracks.rhythm, 4);
	write(m_noteTracks.coop, 5);
	write(m_noteTracks.keys, 6);
	write(m_noteTracks.drums4_pro, 7);
	write(m_noteTracks.drums5, 8);
	write(m_noteTracks.vocals, 9);
	write(m_noteTracks.harmonies, 10);
}
