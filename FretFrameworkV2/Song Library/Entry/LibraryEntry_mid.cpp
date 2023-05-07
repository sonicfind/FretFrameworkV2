#include "LibraryEntry.h"
#include "Notes/Keys.h"
#include "GuitarScan_midi.h"
#include "DrumScan_midi.h"
#include "DrumScan_Legacy_Midi.h"
#include "VocalScan_Midi.h"

void LibraryEntry::scan_mid(const LoadedFile& file)
{
	DrumType_Enum type = DrumType_Enum::LEGACY;
	if (auto fivelane = getModifier("five_lane_drums"))
		type = fivelane->getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;

	VocalScan_Midi::Midi_Scanner_Vocal<3> harmonyTracker;
	MidiFileReader reader(file);
	while (reader.startTrack())
	{
		if (reader.getTrackNumber() > 1 && reader.getEventType() == MidiEventType::Text_TrackName)
		{
			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "PART GUITAR" || name == "T1 GEMS")
				InstrumentalScan_Midi::Scan<GuitarNote<5>>(m_scanTracks.lead_5, reader);
			else if (name == "PART GUITAR GHL")
				InstrumentalScan_Midi::Scan<GuitarNote<6>>(m_scanTracks.lead_6, reader);
			else if (name == "PART BASS")
				InstrumentalScan_Midi::Scan<GuitarNote<5>>(m_scanTracks.bass_5, reader);
			else if (name == "PART BASS GHL")
				InstrumentalScan_Midi::Scan<GuitarNote<6>>(m_scanTracks.bass_6, reader);
			else if (name == "PART RHYTHM")
				InstrumentalScan_Midi::Scan<GuitarNote<5>>(m_scanTracks.rhythm, reader);
			else if (name == "PART GUITAR COOP")
				InstrumentalScan_Midi::Scan<GuitarNote<5>>(m_scanTracks.coop, reader);
			else if (name == "PART KEYS")
				InstrumentalScan_Midi::Scan<Keys<5>>(m_scanTracks.keys, reader);
			else if (name == "PART DRUMS")
			{
				if (type == DrumType_Enum::LEGACY)
				{
					DrumScan_Legacy_Midi drumsLegacy(reader);
					if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
						drumsLegacy.transfer(m_scanTracks.drums4_pro);
					else
						drumsLegacy.transfer(m_scanTracks.drums5);
				}
				else if (type == DrumType_Enum::FOURLANE_PRO)
					InstrumentalScan_Midi::Scan<DrumNote<4, true>>(m_scanTracks.drums4_pro, reader);
				else
					InstrumentalScan_Midi::Scan<DrumNote<5, false>>(m_scanTracks.drums5, reader);
			}
			else if (name == "PART VOCALS")
			{
				VocalScan_Midi::Midi_Scanner_Vocal<1> tracker;
				VocalScan_Midi::Scan<0>(m_scanTracks.vocals, tracker, reader);
			}
			else if (name == "HARM1")
				VocalScan_Midi::Scan<0>(m_scanTracks.harmonies, harmonyTracker, reader);
			else if (name == "HARM2")
				VocalScan_Midi::Scan<1>(m_scanTracks.harmonies, harmonyTracker, reader);
			else if (name == "HARM3")
				VocalScan_Midi::Scan<2>(m_scanTracks.harmonies, harmonyTracker, reader);
		}
	}
}
