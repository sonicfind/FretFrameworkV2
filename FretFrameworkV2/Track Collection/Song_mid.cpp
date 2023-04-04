#include "Song.h"
#include "Tracks/Instrumental/GuitarTrack_midi.h"
#include "Tracks/Instrumental/DrumTrack_midi.h"
#include "Tracks/Vocal/VocalTrack_midi.h"

void Song::load_mid(const std::filesystem::path& path)
{
	MidiFileReader reader(path);

	while (reader.startNextTrack())
	{
		const std::string_view name = reader.extractTextOrSysEx();
		if (reader.getTrackNumber() == 1)
		{
			m_midiSequenceName = UnicodeString::strToU32(name);

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
		else if (name == "EVENTS")
		{
			while (reader.parseEvent())
			{
				if (reader.getEventType() <= MidiEventType::Text_EnumLimit)
				{
					const uint32_t position = reader.getPosition();

					std::string_view text = reader.extractTextOrSysEx();
					if (!addSection_midi(reader.getPosition(), text))
						m_globalEvents.get_or_emplace_back(position).push_back(UnicodeString::strToU32(text));
				}
			}
		}
		else if (name == "PART GUITAR" || name == "T1 GEMS")
			InstrumentTrackMidiParser(m_noteTracks.lead_5, reader);
		else if (name == "PART GUITAR GHL")
			InstrumentTrackMidiParser(m_noteTracks.lead_6, reader);
		else if (name == "PART BASS")
			InstrumentTrackMidiParser(m_noteTracks.bass_5, reader);
		else if (name == "PART BASS GHL")
			InstrumentTrackMidiParser(m_noteTracks.bass_6, reader);
		else if (name == "PART RHYTHM")
			InstrumentTrackMidiParser(m_noteTracks.rhythm, reader);
		else if (name == "PART GUITAR COOP")
			InstrumentTrackMidiParser(m_noteTracks.coop, reader);
		else if (name == "PART KEYS")
			InstrumentTrackMidiParser(m_noteTracks.keys, reader);
		else if (name == "PART DRUMS")
		{
			InstrumentalTrack<DrumNote_Legacy> drumsLegacy;
			InstrumentTrackMidiParser legacyParser(drumsLegacy, reader);
			if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
				m_noteTracks.drums4_pro = std::move(drumsLegacy);
			else
				m_noteTracks.drums5 = std::move(drumsLegacy);
		}
		else if (name == "PART VOCALS")
		{
			VocalTrackMidiParser parser(m_noteTracks.vocals, reader);
			parser.loadFromMidi<0>();
		}
		else
		{
			VocalTrackMidiParser parser(m_noteTracks.harmonies, reader);
			if (name == "HARM1")
				parser.loadFromMidi<0>();
			else if (name == "HARM2")
				parser.loadFromMidi<1>();
			else if (name == "HARM3")
				parser.loadFromMidi<2>();
		}
	}
}

void Song::save_mid(const std::filesystem::path& path)
{

}

bool Song::addSection_midi(uint32_t position, std::string_view str)
{
	static constexpr std::string_view PREFIXES[] = { "[section ", "[prc_" };
	for (std::string_view prefix : PREFIXES)
	{
		if (str.starts_with(prefix))
		{
			m_sectionMarkers.get_or_emplace_back(position) = std::string_view(str.begin() + prefix.length(), str.end() - 1);
			return true;
		}
	}

	return false;
}
