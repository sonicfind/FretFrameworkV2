#include "Song.h"

const std::filesystem::path Song::s_EXTS_CHT[2] = { ".cht", ".chart" };
const std::filesystem::path Song::s_EXTS_MID[2] = { ".mid", ".midi" };
const std::filesystem::path Song::s_EXT_BCH = ".bch";

bool Song::load(const std::filesystem::path& path) noexcept
{
	clear();

	try
	{
		const std::filesystem::path ext = path.extension();
		if (ext == s_EXTS_CHT[0] || ext == s_EXTS_CHT[1])
			load_cht(path);
		else if (ext == s_EXTS_MID[0] || ext == s_EXTS_MID[1])
			load_mid(path);
		else if (ext == s_EXT_BCH)
			load_bch(path);
		else
			throw std::runtime_error(ext.generic_string() + " is not a valid chart type");
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
	for (Track* track : m_noteTracks.array)
		track->clear();
}

bool Song::load_tempoMap(CommonChartParser* parser)
{
	if (!parser->validateSyncTrack())
		return false;

	parser->nextEvent();
	while (parser->isStillCurrentTrack())
	{
		const uint32_t position = parser->parsePosition();
		switch (parser->parseEvent())
		{
		case ChartEvent::BPM:
			m_tempoMarkers.get_or_emplace_back(position) = parser->extractMicrosPerQuarter();
			break;
		case ChartEvent::TIME_SIG:
			m_timeSigs.get_or_emplace_back(position).combine(parser->extractTimeSig());
			break;
		}
		parser->nextEvent();
	}
	return true;
}

bool Song::load_events(CommonChartParser* parser)
{
	if (!parser->validateEventTrack())
		return false;

	parser->nextEvent();
	while (parser->isStillCurrentTrack())
	{
		const uint32_t position = parser->parsePosition();
		switch (parser->parseEvent())
		{
		case ChartEvent::EVENT:
		{
			std::string_view str = parser->extractText();
			if (str.starts_with("section "))
				m_sectionMarkers.get_or_emplace_back(position) = str.substr(8);
			else
				m_globalEvents.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
			break;
		}
		case ChartEvent::SECTION:
			m_sectionMarkers.get_or_emplace_back(position) = parser->extractText();
			break;
		}
		parser->nextEvent();
	}
	return true;
}

bool Song::load_noteTrack(CommonChartParser* parser)
{
	if (!parser->validateNoteTrack())
		return false;

	const size_t index = parser->geNoteTrackID();
	if (index < std::size(m_noteTracks.array))
		m_noteTracks.array[parser->geNoteTrackID()]->load(parser);
	else //BCH only
		parser->skipUnknownTrack();
	return true;
}

void Song::save(CommonChartWriter* writer) const
{
	save_header(writer);
	save_tempoMap(writer);
	save_events(writer);
	save_noteTracks(writer);
}

void Song::save_header(CommonChartWriter* writer) const
{
	writer->writeHeaderTrack(m_tickrate);
	writer->finishTrack();
}

void Song::save_tempoMap(CommonChartWriter* writer) const
{
	writer->writeSyncTrack();
	auto tempo = m_tempoMarkers.begin();
	auto timeSig = m_timeSigs.begin();
	while (tempo != m_tempoMarkers.end() || timeSig != m_timeSigs.end())
	{
		while (tempo != m_tempoMarkers.end() && (timeSig == m_timeSigs.end() || tempo->key <= timeSig->key))
		{
			writer->startEvent(tempo->key, ChartEvent::BPM);
			writer->writeMicrosPerQuarter(**tempo);
			writer->finishEvent();
		}

		while (timeSig != m_timeSigs.end() && (tempo == m_tempoMarkers.end() || timeSig->key < tempo->key))
		{
			writer->startEvent(timeSig->key, ChartEvent::TIME_SIG);
			writer->writeTimeSig(**timeSig);
			writer->finishEvent();
		}
	}
	writer->finishTrack();
}

void Song::save_events(CommonChartWriter* writer) const
{
	struct EventPointer
	{
		const UnicodeString* m_section = nullptr;
		const std::vector<std::u32string>* m_events = nullptr;

		void writeSection(uint32_t position, CommonChartWriter* writer) const
		{
			if (m_section == nullptr)
				return;

			writer->startEvent(position, ChartEvent::SECTION);
			writer->writeText(m_section->toString());
			writer->finishEvent();
		}

		void writeEvents(uint32_t position, CommonChartWriter* writer) const
		{
			if (m_events == nullptr)
				return;

			for (const std::u32string& ev : *m_events)
			{
				writer->startEvent(position, ChartEvent::EVENT);
				writer->writeText(UnicodeString::U32ToStr(ev));
				writer->finishEvent();
			}
		}
	};

	writer->writeEventTrack();

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
	
	writer->finishTrack();
}

void Song::save_noteTracks(CommonChartWriter* writer) const
{
	for (size_t i = 0; i < std::size(m_noteTracks.array); ++i)
	{
		if (m_noteTracks.array[i]->isOccupied())
		{
			writer->writeNoteTrack(i);
			m_noteTracks.array[i]->save(writer);
			writer->finishTrack();
		}
	}
}
