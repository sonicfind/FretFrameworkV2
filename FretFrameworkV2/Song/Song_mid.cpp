#include "Song.h"
#include "Tracks/Instrumental/GuitarTrack_midi.h"
#include "Tracks/Instrumental/DrumTrack_midi.h"
#include "Tracks/Instrumental/Legacy_DrumTrack.h"

void Song::load_mid(const std::filesystem::path& path)
{
	MidiFileReader reader(path);
	while (reader.startNextTrack())
	{
		if (reader.getTrackNumber() == 1)
		{
			if (reader.getEventType() == MidiEventType::Text_TrackName)
				m_midiSequenceName = UnicodeString::strToU32(reader.extractTextOrSysEx());
			load_tempoMap_midi(reader);
		}
		else
		{
			if (reader.getEventType() != MidiEventType::Text_TrackName)
				continue;

			const std::string_view name = reader.extractTextOrSysEx();
			if (name == "EVENTS")
				load_events_midi(reader);
			else if (name == "PART GUITAR" || name == "T1 GEMS")
				m_noteTracks.lead_5.load(reader);
			else if (name == "PART GUITAR GHL")
				m_noteTracks.lead_6.load(reader);
			else if (name == "PART BASS")
				m_noteTracks.bass_5.load(reader);
			else if (name == "PART BASS GHL")
				m_noteTracks.bass_6.load(reader);
			else if (name == "PART RHYTHM")
				m_noteTracks.rhythm.load(reader);
			else if (name == "PART GUITAR COOP")
				m_noteTracks.coop.load(reader);
			else if (name == "PART KEYS")
				m_noteTracks.keys.load(reader);
			else if (name == "PART DRUMS")
			{
				Legacy_DrumTrack drumsLegacy(reader);
				if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
					drumsLegacy.transfer(m_noteTracks.drums4_pro);
				else
					drumsLegacy.transfer(m_noteTracks.drums5);
			}
			else if (name == "PART VOCALS")
				m_noteTracks.vocals.load<0>(reader);
			else if (name == "HARM1")
				m_noteTracks.harmonies.load<0>(reader);
			else if (name == "HARM2")
				m_noteTracks.harmonies.load<1>(reader);
			else if (name == "HARM3")
				m_noteTracks.harmonies.load<2>(reader);
		}
	}
}

void Song::load_tempoMap_midi(MidiFileReader& reader)
{
	while (reader.parseEvent())
	{
		switch (reader.getEventType())
		{
		case MidiEventType::Tempo:
			m_tempoMarkers.get_or_emplace_back(reader.getPosition()) = reader.extractMicrosPerQuarter();
			break;
		case MidiEventType::Time_Sig:
			m_timeSigs.get_or_emplace_back(reader.getPosition()) = reader.extractTimeSig();
			break;
		}
	}
}

void Song::load_events_midi(MidiFileReader& reader)
{
	const auto addSection = [&] (uint32_t position, std::string_view str)
	{
		for (std::string_view prefix : { "[section ", "[prc_" })
		{
			if (str.starts_with(prefix))
			{
				m_sectionMarkers.get_or_emplace_back(position) = std::string_view(str.begin() + prefix.length(), str.end() - 1);
				return true;
			}
		}
		return false;
	};

	while (reader.parseEvent())
	{
		if (reader.getEventType() <= MidiEventType::Text_EnumLimit)
		{
			const uint32_t position = reader.getPosition();

			std::string_view text = reader.extractTextOrSysEx();
			if (!addSection(reader.getPosition(), text))
				m_globalEvents.get_or_emplace_back(position).push_back(UnicodeString::strToU32(text));
		}
	}
}


void Song::save_mid(const std::filesystem::path& path)
{

}
