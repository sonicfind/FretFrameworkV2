#include "LibraryEntry.h"
#include "Notes/Keys.h"
#include "GuitarScan_midi.h"
#include "DrumScan_midi.h"
#include "DrumScan_Legacy_Midi.h"
#include "VocalScan_Midi.h"

void LibraryEntry::scan_mid(const LoadedFile& file)
{
	DrumType_Enum type = getDrumTypeFromModifier();

	Midi_Scanner_Vocal<1> vocalTracker(m_scanTracks.vocals);
	Midi_Scanner_Vocal<3> harmonyTracker(m_scanTracks.harmonies);
	
	MidiFileReader reader(file);
	while (reader.startTrack())
	{
		if (reader.getTrackNumber() > 1 && reader.getEventType() == MidiEventType::Text_TrackName)
		{
			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "PART GUITAR" || name == "T1 GEMS")
				Midi_Scanner<GuitarNote<5>>(m_scanTracks.lead_5, reader);
			else if (name == "PART GUITAR GHL")
				Midi_Scanner<GuitarNote<6>>(m_scanTracks.lead_6, reader);
			else if (name == "PART BASS")
				Midi_Scanner<GuitarNote<5>>(m_scanTracks.bass_5, reader);
			else if (name == "PART BASS GHL")
				Midi_Scanner<GuitarNote<6>>(m_scanTracks.bass_6, reader);
			else if (name == "PART RHYTHM")
				Midi_Scanner<GuitarNote<5>>(m_scanTracks.rhythm, reader);
			else if (name == "PART GUITAR COOP")
				Midi_Scanner<GuitarNote<5>>(m_scanTracks.coop, reader);
			else if (name == "PART KEYS")
				Midi_Scanner<Keys<5>>(m_scanTracks.keys, reader);
			else if (name == "PART DRUMS")
			{
				if (type == DrumType_Enum::LEGACY)
				{
					ScanValues legacyScans;
					DrumScan_Legacy_Midi drumsLegacy(legacyScans, reader);
					if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
						m_scanTracks.drums4_pro.m_subTracks |= legacyScans.m_subTracks;
					else
						m_scanTracks.drums5.m_subTracks |= legacyScans.m_subTracks;
				}
				else if (type == DrumType_Enum::FOURLANE_PRO)
					Midi_Scanner<DrumNote<4, true>>(m_scanTracks.drums4_pro, reader);
				else
					Midi_Scanner<DrumNote<5, false>>(m_scanTracks.drums5, reader);
			}
			else if (name == "PART VOCALS")
				vocalTracker.scan<0>(reader);
			else if (name == "HARM1")
				harmonyTracker.scan<0>(reader);
			else if (name == "HARM2")
				harmonyTracker.scan<1>(reader);
			else if (name == "HARM3")
				harmonyTracker.scan<2>(reader);
		}
	}
}
