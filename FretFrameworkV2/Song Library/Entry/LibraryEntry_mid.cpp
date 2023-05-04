#include "LibraryEntry.h"
#include "GuitarScan_midi.h"
#include "DrumScan_midi.h"
#include "Legacy_DrumScan.h"

void LibraryEntry::scan_mid(const LoadedFile& file)
{
	DrumType_Enum type = DrumType_Enum::LEGACY;
	if (auto fivelane = getModifier("five_lane_drums"))
		type = fivelane->getValue<bool>() ? DrumType_Enum::FIVELANE : DrumType_Enum::FOURLANE_PRO;

	MidiFileReader reader(file);
	std::vector<std::pair<uint32_t, uint32_t>> lyriclines;
	while (reader.startTrack())
	{
		if (reader.getTrackNumber() > 1)
		{
			if (reader.getEventType() != MidiEventType::Text_TrackName)
				continue;

			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "PART GUITAR" || name == "T1 GEMS")
				m_scanTracks.lead_5.scan(reader);
			else if (name == "PART GUITAR GHL")
				m_scanTracks.lead_6.scan(reader);
			else if (name == "PART BASS")
				m_scanTracks.bass_5.scan(reader);
			else if (name == "PART BASS GHL")
				m_scanTracks.bass_6.scan(reader);
			else if (name == "PART RHYTHM")
				m_scanTracks.rhythm.scan(reader);
			else if (name == "PART GUITAR COOP")
				m_scanTracks.coop.scan(reader);
			else if (name == "PART KEYS")
				m_scanTracks.keys.scan(reader);
			else if (name == "PART DRUMS")
			{
				if (type == DrumType_Enum::LEGACY)
				{
					Legacy_DrumScan drumsLegacy(reader);
					if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
						drumsLegacy.transfer(m_scanTracks.drums4_pro);
					else
						drumsLegacy.transfer(m_scanTracks.drums5);
				}
				else if (type == DrumType_Enum::FOURLANE_PRO)
					m_scanTracks.drums4_pro.scan(reader);
				else
					m_scanTracks.drums5.scan(reader);
			}
			else if (name == "PART VOCALS")
			{
				static std::vector<std::pair<uint32_t, uint32_t>> dummy;
				m_scanTracks.vocals.scan(reader, dummy);
			}
			else if (name == "HARM1")
				m_scanTracks.harmonies.scan<0>(reader, lyriclines);
			else if (name == "HARM2")
				m_scanTracks.harmonies.scan<1>(reader, lyriclines);
			else if (name == "HARM3")
				m_scanTracks.harmonies.scan<2>(reader, lyriclines);
		}
	}
}
