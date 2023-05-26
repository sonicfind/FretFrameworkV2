#include "Song.h"
#include "Tracks/Instrumental/Extended_Loader_Instrument.h"
#include "Tracks/Vocal/Extended_Loader_Vocals.h"

void Song::traverse(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
		{
			const auto LoadTrack = [&](size_t index)
			{
				switch (index)
				{
				case 0:  return Extended_Load_Instrument::Load<GuitarNote<5>>(m_noteTracks.lead_5, parser);
				case 1:  return Extended_Load_Instrument::Load<GuitarNote<6>>(m_noteTracks.lead_6, parser);
				case 2:  return Extended_Load_Instrument::Load<GuitarNote<5>>(m_noteTracks.bass_5, parser);
				case 3:  return Extended_Load_Instrument::Load<GuitarNote<6>>(m_noteTracks.bass_6, parser);
				case 4:  return Extended_Load_Instrument::Load<GuitarNote<5>>(m_noteTracks.rhythm, parser);
				case 5:  return Extended_Load_Instrument::Load<GuitarNote<5>>(m_noteTracks.coop, parser);
				case 6:  return Extended_Load_Instrument::Load<Keys<5>>(m_noteTracks.keys, parser);
				case 7:  return Extended_Load_Instrument::Load<DrumNote<DrumPad_Pro, 4>>(m_noteTracks.drums4_pro, parser);
				case 8:  return Extended_Load_Instrument::Load<DrumNote<DrumPad, 5>>(m_noteTracks.drums5, parser);
				case 9:  return Extended_Load_Vocals::Load<1>(m_noteTracks.vocals, parser);
				case 10: return Extended_Load_Vocals::Load<3>(m_noteTracks.harmonies, parser);
				default:
					return false;
				}
			};

			if (!LoadTrack(parser.geNoteTrackID()))
				parser.skipTrack();
		}
		else if (parser.validateSyncTrack())
			load_tempoMap(parser);
		else if (parser.validateEventTrack())
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
						m_events.sections.get_or_emplace_back(trackEvent.first) = str.substr(8);
					else
						m_events.globals.get_or_emplace_back(trackEvent.first).push_back(UnicodeString::strToU32(str));
					break;
				}
				case ChartEvent::SECTION:
					m_events.sections.get_or_emplace_back(trackEvent.first) = parser.extractText();
					break;
				}
				parser.nextEvent();
			}
		}
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
			m_sync.tempoMarkers.get_or_emplace_back(trackEvent.first).first = parser.extractMicrosPerQuarter();
			break;
		case ChartEvent::ANCHOR:
			m_sync.tempoMarkers.get_or_emplace_back(trackEvent.first).second = parser.extractAnchor();
			break;
		case ChartEvent::TIME_SIG:
			m_sync.timeSigs.get_or_emplace_back(trackEvent.first) = parser.extractTimeSig();
			break;
		}
		parser.nextEvent();
	}
}
