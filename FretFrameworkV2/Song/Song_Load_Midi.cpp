#include "Song.h"
#include "Tracks/Instrumental/Midi_Loader_Guitar.h"
#include "Tracks/Instrumental/Midi_Loader_Drums.h"
#include "Tracks/Instrumental/DrumTrack_Transfer.h"
#include "Tracks/Vocal/Midi_Loader_Vocals.h"
#include "Tracks/Instrumental/Midi_Loader_ProGuitar.h"
#include "Tracks/Instrumental/Midi_Loader_ProKeys.h"
#include <iostream>

bool AddSection(SimpleFlatMap<UnicodeString>& sections, const uint64_t position, const std::string_view str)
{
	for (std::string_view prefix : { "[section ", "[prc_" })
	{
		if (str.starts_with(prefix))
		{
			sections.get_or_emplace_back(position) = std::string_view(str.begin() + prefix.length(), str.end() - 1);
			return true;
		}
	}
	return false;
}

void Song::load_mid(const std::filesystem::path& path)
{
	Midi_Loader_Vocal<1> vocalTracker(m_noteTracks.vocals, m_multiplier_note);
	Midi_Loader_Vocal<3> harmonyTracker(m_noteTracks.harmonies, m_multiplier_note);

	MidiFileReader reader(path, m_multiplier_note);
	m_tickrate = reader.getTickRate();
	setSustainThreshold();

	const auto readTrack = [&] (const std::string_view name)
	{
		if (name == "EVENTS")
		{
			if (!m_events.globals.isEmpty() || !m_events.sections.isEmpty())
				return false;

			while (reader.tryParseEvent())
			{
				MidiEvent midiEvent = reader.getEvent();
				if (midiEvent.type <= MidiEventType::Text_EnumLimit)
				{
					std::string_view text = reader.extractTextOrSysEx();
					if (!AddSection(m_events.sections, midiEvent.position, text))
						m_events.globals.get_or_emplace_back(midiEvent.position).push_back(UnicodeString::strToU32(text));
				}
			}
		}
		else if (name == "PART GUITAR" || name == "T1 GEMS") return Midi_Loader_Instrument::Load(m_noteTracks.lead_5, reader);
		else if (name == "PART GUITAR GHL")                  return Midi_Loader_Instrument::Load(m_noteTracks.lead_6, reader);
		else if (name == "PART BASS")                        return Midi_Loader_Instrument::Load(m_noteTracks.bass_5, reader);
		else if (name == "PART BASS GHL")                    return Midi_Loader_Instrument::Load(m_noteTracks.bass_6, reader);
		else if (name == "PART RHYTHM")                      return Midi_Loader_Instrument::Load(m_noteTracks.rhythm, reader);
		else if (name == "PART GUITAR COOP")                 return Midi_Loader_Instrument::Load(m_noteTracks.coop, reader);
		else if (name == "PART KEYS")                        return Midi_Loader_Instrument::Load(m_noteTracks.keys, reader);
		else if (name == "PART DRUMS")
		{
			if (m_baseDrumType == DrumType_Enum::LEGACY)
			{
				DrumNote_Legacy::ResetType();
				InstrumentalTrack<DrumNote_Legacy> track;
				if (!Midi_Loader_Instrument::Load(track, reader))
					return false;

				if (DrumNote_Legacy::GetType() != DrumType_Enum::FIVELANE) LegacyDrums::Transfer(track, m_noteTracks.drums4_pro);
				else                                                       LegacyDrums::Transfer(track, m_noteTracks.drums5);
			}
			else if (m_baseDrumType == DrumType_Enum::FOURLANE_PRO) return Midi_Loader_Instrument::Load(m_noteTracks.drums4_pro, reader);
			else                                                    return Midi_Loader_Instrument::Load(m_noteTracks.drums5, reader);
		}
		else if (name == "PART VOCALS")                   return vocalTracker.load(reader);
		else if (name == "HARM1" || name == "PART HARM1") return harmonyTracker.load<0>(reader);
		else if (name == "HARM2" || name == "PART HARM2") return harmonyTracker.load<1>(reader);
		else if (name == "HARM3" || name == "PART HARM3") return harmonyTracker.load<2>(reader);
		else if (name == "PART REAL_GUITAR")              return Midi_Loader_Instrument::Load(m_noteTracks.proGuitar_17, reader);
		else if (name == "PART REAL_GUITAR_22")           return Midi_Loader_Instrument::Load(m_noteTracks.proGuitar_22, reader);
		else if (name == "PART REAL_BASS")              return Midi_Loader_Instrument::Load(m_noteTracks.proBass_17, reader);
		else if (name == "PART REAL_BASS_22")           return Midi_Loader_Instrument::Load(m_noteTracks.proBass_22, reader);
		else if (name.starts_with("PART REAL_KEYS_"))
		{
			size_t index = 4;
			switch (name.back())
			{
			case 'X': index = 3; break;
			case 'H': index = 2; break;
			case 'M': index = 1; break;
			case 'E': index = 0; break;
			}

			if (index > 3 || m_noteTracks.proKeys[index].isOccupied())
				return false;

			InstrumentalTrack<Keys_Pro> proKeys_buffer;
			Midi_Loader_Instrument::Load(proKeys_buffer, reader);

			m_noteTracks.proKeys[index].m_notes = std::move(proKeys_buffer[0].m_notes);
			m_noteTracks.proKeys[index].m_ranges = std::move(proKeys_buffer[0].m_ranges);
			if (m_noteTracks.proKeys.m_specialPhrases.isEmpty())
				m_noteTracks.proKeys.m_specialPhrases = std::move(proKeys_buffer.m_specialPhrases);

			if (m_noteTracks.proKeys.m_events.isEmpty())
				m_noteTracks.proKeys.m_events = std::move(proKeys_buffer.m_events);
		}
		return true;
	};

	while (reader.startTrack())
	{
		if (reader.getTrackNumber() == 1)
		{
			if (reader.getEvent().type == MidiEventType::Text_TrackName)
				m_midiSequenceName = UnicodeString::strToU32(reader.extractTextOrSysEx());

			while (reader.tryParseEvent())
			{
				MidiEvent midiEvent = reader.getEvent();
				switch (midiEvent.type)
				{
				case MidiEventType::Tempo:
					m_sync.tempoMarkers.get_or_emplace_back(midiEvent.position).first = reader.extractMicrosPerQuarter();
					break;
				case MidiEventType::Time_Sig:
					m_sync.timeSigs.get_or_emplace_back(midiEvent.position) = reader.extractTimeSig();
					break;
				}
			}
		}
		else if (reader.getEvent().type == MidiEventType::Text_TrackName)
		{
			const std::string_view name = reader.extractTextOrSysEx();
			if (!readTrack(name))
				std::cout << "Track " << name << " failed to load or was already loaded previously\n";
		}
	}

	if (m_noteTracks.drums4_pro.isOccupied() && !getModifier("pro_drums"))
		m_modifiers.push_back({ "pro_drums", false });
}
