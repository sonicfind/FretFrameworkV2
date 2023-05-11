#include "Song.h"
#include "Tracks/Instrumental/Midi_Saver_Guitar.h"
#include "Tracks/Instrumental/Midi_Saver_Drums.h"
#include "Tracks/Vocal/Midi_Saver_Vocals.h"

template <class T>
void Save(const InstrumentalTrack<T>& track, std::string_view name, MidiFileWriter& writer)
{
	if (track.isOccupied())
	{
		writer.startTrack(name);
		Midi_Saver_Instrument::Save(track, writer);
		writer.finishTrack();
	}
}

template <size_t INDEX, size_t numTracks>
void Save(const VocalTrack<numTracks>& track, std::string_view name, MidiFileWriter& writer)
{
	if (track.isOccupied(INDEX))
	{
		writer.startTrack(name);
		Midi_Saver_Vocals::Save<INDEX>(track, writer);
		writer.finishTrack();
	}
}

void WriteTempoMap(const SyncTrack& track, const std::u32string& midiSequence, MidiFileWriter& writer);
void WriteEvents(const SongEvents& track, MidiFileWriter& writer);

void Song::save_mid(const std::filesystem::path& path) const
{
	MidiFileWriter writer(path, m_tickrate);
	WriteTempoMap(m_sync, m_midiSequenceName, writer);
	WriteEvents(m_events, writer);
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

void WriteTempoMap(const SyncTrack& track, const std::u32string& midiSequence, MidiFileWriter& writer)
{
	auto sigIter = track.timeSigs.begin();
	auto tempoIter = track.tempoMarkers.begin();
	bool sigValid = sigIter != track.timeSigs.end();
	bool tempoValid = tempoIter != track.tempoMarkers.end();

	writer.startTrack(UnicodeString::U32ToStr(midiSequence));
	TimeSig currSig = { 4, 2, 24, 8 };
	while (sigValid || tempoValid)
	{
		while (sigValid && (!tempoValid || sigIter->key <= tempoIter->key))
		{
			TimeSig timeSig = sigIter->object;
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
			writer.writeTimeSig(sigIter->key, timeSig);
			sigValid = ++sigIter != track.timeSigs.end();
		}

		while (tempoValid && (!sigValid || tempoIter->key < sigIter->key))
		{
			writer.writeMicros(tempoIter->key, tempoIter->object.first);
			tempoValid = ++tempoIter != track.tempoMarkers.end();
		}
	}
	writer.finishTrack();
}

void WriteEvents(const SongEvents& track, MidiFileWriter& writer)
{
	auto sectionIter = track.sections.begin();
	auto eventIter = track.globals.begin();
	bool sectionValid = sectionIter != track.sections.end();
	bool eventValid = eventIter != track.globals.end();

	writer.startTrack("EVENTS");
	while (sectionValid || eventValid)
	{
		while (sectionValid && (!eventValid || sectionIter->key <= eventIter->key))
		{
			writer.writeText(sectionIter->key, "[section " + sectionIter->object.toString() + ']');
			sectionValid = ++sectionIter != track.sections.end();
		}

		while (eventValid && (!sectionValid || eventIter->key < sectionIter->key))
		{
			for (const auto& str : eventIter->object)
				writer.writeText(eventIter->key, UnicodeString::U32ToStr(str));
			eventValid = ++eventIter != track.globals.end();
		}
	}
	writer.finishTrack();
}
