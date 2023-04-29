#include "CacheEntry.h"
#include "GuitarScan_midi.h"
#include "DrumScan_midi.h"
#include "Legacy_DrumScan.h"

void CacheEntry::scan_mid(const std::filesystem::path& path)
{
	MidiFileReader reader(path);
	std::vector<std::pair<uint32_t, uint32_t>> lyriclines;
	while (reader.startNextTrack())
	{
		if (reader.getTrackNumber() > 1)
		{
			if (reader.getEventType() != MidiEventType::Text_TrackName)
				continue;

			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "PART GUITAR" || name == "T1 GEMS")
				m_noteTracks.lead_5.scan(reader);
			else if (name == "PART GUITAR GHL")
				m_noteTracks.lead_6.scan(reader);
			else if (name == "PART BASS")
				m_noteTracks.bass_5.scan(reader);
			else if (name == "PART BASS GHL")
				m_noteTracks.bass_6.scan(reader);
			else if (name == "PART RHYTHM")
				m_noteTracks.rhythm.scan(reader);
			else if (name == "PART GUITAR COOP")
				m_noteTracks.coop.scan(reader);
			else if (name == "PART KEYS")
				m_noteTracks.keys.scan(reader);
			else if (name == "PART DRUMS")
			{
				Legacy_DrumScan drumsLegacy(reader);
				if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
					drumsLegacy.transfer(m_noteTracks.drums4_pro);
				else
					drumsLegacy.transfer(m_noteTracks.drums5);
			}
			else if (name == "PART VOCALS")
			{
				static std::vector<std::pair<uint32_t, uint32_t>> dummy;
				m_noteTracks.vocals.scan(reader, dummy);
			}
			else if (name == "HARM1")
				m_noteTracks.harmonies.scan<0>(reader, lyriclines);
			else if (name == "HARM2")
				m_noteTracks.harmonies.scan<1>(reader, lyriclines);
			else if (name == "HARM3")
				m_noteTracks.harmonies.scan<2>(reader, lyriclines);
		}
	}
}