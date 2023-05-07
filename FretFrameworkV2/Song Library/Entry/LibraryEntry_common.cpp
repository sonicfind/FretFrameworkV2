#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "DrumScan_Extended.h"
#include "VocalScan_Extended.h"

void LibraryEntry::traverse(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
			scan_noteTrack(parser);
		else
			parser.skipTrack();
	}
}

void LibraryEntry::scan_noteTrack(CommonChartParser& parser)
{
	const auto ScanTrack = [&](size_t index)
	{
		switch (index)
		{
		case 0:  return InstrumentalScan_Extended::Scan<GuitarNote<5>>(m_scanTracks.lead_5, parser);
		case 1:  return InstrumentalScan_Extended::Scan<GuitarNote<6>>(m_scanTracks.lead_6, parser);
		case 2:  return InstrumentalScan_Extended::Scan<GuitarNote<5>>(m_scanTracks.bass_5, parser);
		case 3:  return InstrumentalScan_Extended::Scan<GuitarNote<6>>(m_scanTracks.bass_6, parser);
		case 4:  return InstrumentalScan_Extended::Scan<GuitarNote<5>>(m_scanTracks.rhythm, parser);
		case 5:  return InstrumentalScan_Extended::Scan<GuitarNote<5>>(m_scanTracks.coop, parser);
		case 6:  return InstrumentalScan_Extended::Scan<Keys<5>>(m_scanTracks.keys, parser);
		case 7:  return InstrumentalScan_Extended::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, parser);
		case 8:  return InstrumentalScan_Extended::Scan<DrumNote<5, false>>(m_scanTracks.drums5, parser);
		case 9:  return VocalScan_Extended::Scan<1>(m_scanTracks.vocals, parser);
		case 10: return VocalScan_Extended::Scan<3>(m_scanTracks.harmonies, parser);
		default:
			return false;
		}
	};

	if (!ScanTrack(parser.geNoteTrackID()))
		parser.skipTrack();
}
