#include "LibraryEntry.h"
#include "Midi_Scan_Guitar.h"
#include "Midi_Scan_Drums.h"
#include "Notes/Keys.h"
#include "Midi_Scan_Vocals.h"
#include "Midi_Scan_ProGuitar.h"

void LibraryEntry::scan_mid(const LoadedFile& file)
{
	InstrumentScan<DrumNote_Legacy> legacyScans(getDrumTypeFromModifier());
	Midi_Scanner_Vocal::Scanner_Vocal<1> vocalTracker(m_scanTracks.vocals);
	Midi_Scanner_Vocal::Scanner_Vocal<3> harmonyTracker(m_scanTracks.harmonies);
	
	MidiFileReader reader(file);
	while (reader.startTrack())
	{
		if (reader.getTrackNumber() > 1 && reader.getEventType() == MidiEventType::Text_TrackName)
		{
			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "PART GUITAR" || name == "T1 GEMS")
				Midi_Scanner_Instrument::Scan(m_scanTracks.lead_5, reader);
			else if (name == "PART GUITAR GHL")
				Midi_Scanner_Instrument::Scan(m_scanTracks.lead_6, reader);
			else if (name == "PART BASS")
				Midi_Scanner_Instrument::Scan(m_scanTracks.bass_5, reader);
			else if (name == "PART BASS GHL")
				Midi_Scanner_Instrument::Scan(m_scanTracks.bass_6, reader);
			else if (name == "PART RHYTHM")
				Midi_Scanner_Instrument::Scan(m_scanTracks.rhythm, reader);
			else if (name == "PART GUITAR COOP")
				Midi_Scanner_Instrument::Scan(m_scanTracks.coop, reader);
			else if (name == "PART KEYS")
				Midi_Scanner_Instrument::Scan(m_scanTracks.keys, reader);
			else if (name == "PART DRUMS")
			{
				if (legacyScans.getType() == DrumType_Enum::LEGACY)
				{
					
					if (!Midi_Scanner_Instrument::Scan(legacyScans, reader))
						continue;

					if (legacyScans.getType() != DrumType_Enum::FIVELANE)
						m_scanTracks.drums4_pro = legacyScans;
					else
						m_scanTracks.drums5 = legacyScans;
				}
				else if (legacyScans.getType() == DrumType_Enum::FOURLANE_PRO)
					Midi_Scanner_Instrument::Scan(m_scanTracks.drums4_pro, reader);
				else
					Midi_Scanner_Instrument::Scan(m_scanTracks.drums5, reader);
			}
			else if (name == "PART VOCALS")
				vocalTracker.scan(reader);
			else if (name == "HARM1")
				harmonyTracker.scan<0>(reader);
			else if (name == "HARM2")
				harmonyTracker.scan<1>(reader);
			else if (name == "HARM3")
				harmonyTracker.scan<2>(reader);
			else if (name == "PART REAL_GUITAR")
				Midi_Scanner_Instrument::Scan(m_scanTracks.proguitar_17, reader);
			else if (name == "PART REAL_GUITAR_22")
				Midi_Scanner_Instrument::Scan(m_scanTracks.proguitar_22, reader);
		}
	}

	if (m_scanTracks.drums4_pro.getSubTracks() > 0 && !getModifier("pro_drums"))
		m_modifiers.push_back({ "pro_drums", false });
}
