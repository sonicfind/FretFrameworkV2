#pragma once
#include "Tracks/Instrumental/InstrumentalTrack.h"
#include "Tracks/Vocal/VocalTrack.h"
#include "Song Library/Entry/LibraryEntry.h"
#include "MicrosPerQuarter.h"

enum class EntryStatus
{
	UNCHANGED,
	NEEDS_RESCAN,
	ERROR
};

class Song
{
public:
	EntryStatus load(const LibraryEntry& entry);
	bool load(const std::pair<std::filesystem::path, ChartType>& chartFile) noexcept;
	bool save(std::filesystem::path path) noexcept;
	void resetTempoMap();
	void clear();

private:
	void setMetaData(const LibraryEntry& entry);
	void setMetaData(const std::filesystem::path& iniFile);

	bool loadIni(const std::filesystem::path& iniFile);

	void load_cht(const std::filesystem::path& path);
	void load_mid(const std::filesystem::path& path);
	void load_bch(const std::filesystem::path& path);
	

	void traverse(CommonChartParser& parser);
	void load_tempoMap(CommonChartParser& parser);
	void load_events(CommonChartParser& parser);
	void load_noteTrack(CommonChartParser& parser);

	int16_t load_header_cht(ChtFileReader& reader);

	void traverse_cht_V1(ChtFileReader& reader);
	void load_events_V1(ChtFileReader& reader);

	void load_tempoMap_midi(MidiFileReader& reader);
	void load_events_midi(MidiFileReader& reader);

	void save_cht(const std::filesystem::path& path);
	void save_bch(const std::filesystem::path& path);
	void save_mid(const std::filesystem::path& path);

	void save(CommonChartWriter& writer) const;
	void save_header(CommonChartWriter& writer) const;
	void save_tempoMap(CommonChartWriter& writer) const;
	void save_events(CommonChartWriter& writer) const;
	void save_noteTracks(CommonChartWriter& writer) const;

private:
	std::u32string m_name;
	std::u32string m_artist;
	std::u32string m_album;
	std::u32string m_genre;
	std::u32string m_year;
	std::u32string m_charter;
	std::u32string m_playlist;

	uint32_t m_hopo_frequency = 0;
	unsigned char m_multiplier_note = 116;
	uint32_t m_sustain_cutoff_threshold = 0;
	bool m_eighthnote_hopo = false;
	DrumType_Enum m_baseDrumType = DrumType_Enum::LEGACY;

	std::vector<Modifiers::Modifier> m_modifiers;

	struct
	{
		InstrumentalTrack_Extended<GuitarNote<5>>      lead_5;
		InstrumentalTrack_Extended<GuitarNote<6>>      lead_6;
		InstrumentalTrack_Extended<GuitarNote<5>>      bass_5;
		InstrumentalTrack_Extended<GuitarNote<6>>      bass_6;
		InstrumentalTrack_Extended<GuitarNote<5>>      rhythm;
		InstrumentalTrack_Extended<GuitarNote<5>>      coop;
		InstrumentalTrack_Extended<Keys<5>>            keys;
		InstrumentalTrack_Extended<DrumNote<4, true>>  drums4_pro;
		InstrumentalTrack_Extended<DrumNote<5, false>> drums5;
		VocalTrack<1>                                  vocals;
		VocalTrack<3>                                  harmonies;
	} m_noteTracks;

	uint32_t m_tickrate = 192;
	SimpleFlatMap<MicrosPerQuarter> m_tempoMarkers;
	SimpleFlatMap<TimeSig> m_timeSigs;
	SimpleFlatMap<UnicodeString> m_sectionMarkers;
	SimpleFlatMap<std::vector<std::u32string>> m_globalEvents;

	std::u32string m_midiSequenceName;
};
