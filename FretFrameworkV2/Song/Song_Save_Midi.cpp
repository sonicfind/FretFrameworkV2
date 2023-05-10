#include "Song.h"
#include "Tracks/Instrumental/Midi_Saver_Guitar.h"
#include "Tracks/Instrumental/Midi_Saver_Drums.h"
#include "Tracks/Vocal/Midi_Saver_Vocals.h"

template <class T>
void Save(const InstrumentalTrack<T>& track, std::string_view name, MidiFileWriter& writer)
{
	if (track.isOccupied())
	{
		writer.setTrackName(name);
		Midi_Saver_Instrument::Save(track, writer);
		writer.writeTrack();
	}
}

template <size_t INDEX, size_t numTracks>
void Save(const VocalTrack<numTracks>& track, std::string_view name, MidiFileWriter& writer)
{
	if (track.isOccupied(INDEX))
	{
		writer.setTrackName(name);
		Midi_Saver_Vocals::Save<INDEX>(track, writer);
		writer.writeTrack();
	}
}

void Song::save_mid(const std::filesystem::path& path) const
{
	MidiFileWriter writer(path, m_tickrate);

	if (!m_midiSequenceName.empty())
		writer.setTrackName(UnicodeString::U32ToStr(m_midiSequenceName));

	{
		TimeSig currSig = { 4, 2, 24, 8 };
		for (const auto& node : m_sync.timeSigs)
		{
			TimeSig timeSig = *node;
			if (timeSig.numerator == 0)
				timeSig.numerator = currSig.numerator;
			else
				currSig.numerator = timeSig.numerator;

			if (timeSig.denominator <= 6)
				currSig.denominator = timeSig.denominator;
			else
				timeSig.denominator = currSig.denominator;

			if (timeSig.metronome == 0)
				timeSig.metronome = currSig.metronome;
			else
				currSig.metronome = timeSig.metronome;

			if (timeSig.num32nds == 0)
				timeSig.num32nds = currSig.num32nds;
			else
				currSig.num32nds = timeSig.num32nds;
			writer.addTimeSig(node.key, timeSig);
		}
	}

	for (const auto& tempo : m_sync.tempoMarkers)
		writer.addMicros(tempo.key, tempo->first);
	writer.writeTrack();
	

	writer.setTrackName("EVENTS");
	for (const auto& section : m_events.sections)
		writer.addText(section.key, "[section " + section->toString() + ']');

	for (const auto& vec : m_events.globals)
		for (const auto& ev : *vec)
			writer.addText(vec.key, UnicodeString::U32ToStr(ev));
	writer.writeTrack();


	Save(m_noteTracks.lead_5      , "PART GUITAR", writer);
	Save(m_noteTracks.lead_6      , "PART GUITAR GHL", writer);
	Save(m_noteTracks.bass_5      , "PART BASS", writer);
	Save(m_noteTracks.bass_6      , "PART BASS GHL", writer);
	Save(m_noteTracks.rhythm      , "PART RHYTHM", writer);
	Save(m_noteTracks.coop        , "PART GUITAR COOP", writer);
	Save(m_noteTracks.keys        , "PART KEYS", writer);
	Save(m_noteTracks.drums4_pro  , "PART DRUMS", writer);
	Save(m_noteTracks.drums5      , "PART DRUMS", writer);
	Save<0>(m_noteTracks.vocals   , "PART VOCALS", writer);
	Save<0>(m_noteTracks.harmonies, "HARM1", writer);
	Save<1>(m_noteTracks.harmonies, "HARM2", writer);
	Save<2>(m_noteTracks.harmonies, "HARM3", writer);
}
