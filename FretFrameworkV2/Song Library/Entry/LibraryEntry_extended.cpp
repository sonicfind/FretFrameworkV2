#include "LibraryEntry.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Extended_Scan_Drums.h"
#include "Extended_Scan_Vocals.h"

void LibraryEntry::traverse(CommonChartParser& parser)
{
	while (parser.isStartOfTrack())
	{
		if (parser.validateNoteTrack())
		{
			const auto ScanTrack = [&](size_t index)
			{
				switch (index)
				{
				case 0:  return Extended_Scan::Scan<GuitarNote<5>>(m_scanTracks.lead_5, parser);
				case 1:  return Extended_Scan::Scan<GuitarNote<6>>(m_scanTracks.lead_6, parser);
				case 2:  return Extended_Scan::Scan<GuitarNote<5>>(m_scanTracks.bass_5, parser);
				case 3:  return Extended_Scan::Scan<GuitarNote<6>>(m_scanTracks.bass_6, parser);
				case 4:  return Extended_Scan::Scan<GuitarNote<5>>(m_scanTracks.rhythm, parser);
				case 5:  return Extended_Scan::Scan<GuitarNote<5>>(m_scanTracks.coop, parser);
				case 6:  return Extended_Scan::Scan<Keys<5>>(m_scanTracks.keys, parser);
				case 7:  return Extended_Scan::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, parser);
				case 8:  return Extended_Scan::Scan<DrumNote<5, false>>(m_scanTracks.drums5, parser);
				case 9:  return Extended_Scan_Vocals::Scan<1>(m_scanTracks.vocals, parser);
				case 10: return Extended_Scan_Vocals::Scan<3>(m_scanTracks.harmonies, parser);
				default:
					return false;
				}
			};

			if (!ScanTrack(parser.geNoteTrackID()))
				parser.skipTrack();
		}
		else
			parser.skipTrack();
	}
}
