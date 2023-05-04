#include "Song.h"
#include "Tracks/Instrumental/GuitarTrack_midi.h"
#include "Tracks/Instrumental/DrumTrack_midi.h"
#include "Tracks/Instrumental/Legacy_DrumTrack.h"

void Song::load_mid(const std::filesystem::path& path)
{
	MidiFileReader reader(path, m_multiplier_note);
	m_tickrate = reader.getTickRate();
	setSustainThreshold();

	while (reader.startTrack())
	{
		if (reader.getTrackNumber() == 1)
		{
			if (reader.getEventType() == MidiEventType::Text_TrackName)
				m_midiSequenceName = UnicodeString::strToU32(reader.extractTextOrSysEx());
			load_tempoMap_midi(reader);
		}
		else if (reader.getEventType() == MidiEventType::Text_TrackName)
		{
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
				if (m_baseDrumType == DrumType_Enum::LEGACY)
				{
					Legacy_DrumTrack drumsLegacy(reader);
					if (drumsLegacy.getDrumType() != DrumType_Enum::FIVELANE)
						drumsLegacy.transfer(m_noteTracks.drums4_pro);
					else
						drumsLegacy.transfer(m_noteTracks.drums5);
				}
				else if (m_baseDrumType == DrumType_Enum::FOURLANE_PRO)
					m_noteTracks.drums4_pro.load(reader);
				else
					m_noteTracks.drums5.load(reader);
			}
			else if (name == "PART VOCALS")
				m_noteTracks.vocals.load(reader);
			else if (name == "HARM1" || name == "PART HARM1")
				m_noteTracks.harmonies.load<0>(reader);
			else if (name == "HARM2" || name == "PART HARM2")
				m_noteTracks.harmonies.load<1>(reader);
			else if (name == "HARM3" || name == "PART HARM3")
				m_noteTracks.harmonies.load<2>(reader);
		}
	}
}

void Song::load_tempoMap_midi(MidiFileReader& reader)
{
	while (const auto midiEvent = reader.parseEvent())
	{
		switch (midiEvent->type)
		{
		case MidiEventType::Tempo:
			m_tempoMarkers.get_or_emplace_back(midiEvent->position) = reader.extractMicrosPerQuarter();
			break;
		case MidiEventType::Time_Sig:
			m_timeSigs.get_or_emplace_back(midiEvent->position) = reader.extractTimeSig();
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

	while (const auto midiEvent = reader.parseEvent())
	{
		if (midiEvent->type <= MidiEventType::Text_EnumLimit)
		{
			std::string_view text = reader.extractTextOrSysEx();
			if (!addSection(midiEvent->position, text))
				m_globalEvents.get_or_emplace_back(midiEvent->position).push_back(UnicodeString::strToU32(text));
		}
	}
}


void Song::save_mid(const std::filesystem::path& path) const
{
	MidiFileWriter writer(path, m_tickrate);
	save_tempoMap(writer);
	save_events(writer);

	m_noteTracks.lead_5    .save(writer, "PART GUITAR");
	m_noteTracks.lead_6    .save(writer, "PART GUITAR GHL");
	m_noteTracks.bass_5    .save(writer, "PART BASS");
	m_noteTracks.bass_6    .save(writer, "PART BASS GHL");
	m_noteTracks.rhythm    .save(writer, "PART RHYTHM");
	m_noteTracks.coop      .save(writer, "PART GUITAR COOP");
	m_noteTracks.keys      .save(writer, "PART KEYS");
	m_noteTracks.drums4_pro.save(writer, "PART DRUMS");
	m_noteTracks.drums5    .save(writer, "PART DRUMS");
	m_noteTracks.vocals    .save(writer, "PART VOCALS");
	m_noteTracks.harmonies .save<0>(writer, "HARM1");
	m_noteTracks.harmonies .save<1>(writer, "HARM2");
	m_noteTracks.harmonies .save<2>(writer, "HARM3");
}

void Song::save_tempoMap(MidiFileWriter& writer) const
{
	if (!m_midiSequenceName.empty())
		writer.setTrackName(UnicodeString::U32ToStr(m_midiSequenceName));

	for (const auto& timeSig : m_timeSigs)
		writer.addTimeSig(timeSig.key, *timeSig);

	for (const auto& tempo : m_tempoMarkers)
		writer.addMicros(tempo.key, *tempo);
	writer.writeTrack();
}

void Song::save_events(MidiFileWriter& writer) const
{
	writer.setTrackName("EVENTS");
	for (const auto& section : m_sectionMarkers)
			writer.addText(section.key, "[section " + section->toString() + ']');

	for (const auto& vec : m_globalEvents)
		for (const auto& ev : *vec)
			writer.addText(vec.key, UnicodeString::U32ToStr(ev));
	writer.writeTrack();
}
