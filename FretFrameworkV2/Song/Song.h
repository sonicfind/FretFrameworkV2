#pragma once
#include "Tracks/Instrumental/InstrumentalTrack.h"
#include "Tracks/Vocal/VocalTrack.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Notes/DrumNote.h"
#include "SyncTrack.h"
#include "SongEvents.h"
#include "Song Library/Entry/LibraryEntry.h"

enum class EntryStatus
{
	VALID,
	NEEDS_RESCAN,
	ERROR
};

class Song
{
public:
	EntryStatus load(const LibraryEntry& entry);
	bool load(const std::pair<std::filesystem::path, ChartType>& chartFile) noexcept;
	bool save(ChartType type) const noexcept;
	void clear();

private:
	void setMetaData(const LibraryEntry& entry);
	void setMetaData();
	void setSustainThreshold() const;
	void checkStartOfTempoMap();

	bool loadIni();
	void saveIni() const;
	void validateAudioStreams(const std::filesystem::path& directory);

	void load_cht(const std::filesystem::path& path, bool isFull);
	void load_mid(const std::filesystem::path& path);
	void load_bch(const std::filesystem::path& path);
	

	void traverse(CommonChartParser& parser);
	void load_tempoMap(CommonChartParser& parser);

	int16_t load_header_cht(ChtFileReader& reader);
	int16_t load_header_cht_basic(ChtFileReader& reader);

	void traverse_cht_V1(ChtFileReader& reader);

	void save_extended(const std::filesystem::path& path, bool binary) const;
	void save_mid(const std::filesystem::path& path) const;

private:
	PointerWrapper<Modifiers::Modifier> getModifier(std::string_view name) noexcept;
	uint64_t getHopoThreshold()  const noexcept;

private:
	std::filesystem::path m_directory;

	std::u32string m_name;
	std::u32string m_artist;
	std::u32string m_album;
	std::u32string m_genre;
	std::u32string m_year;
	std::u32string m_charter;
	std::u32string m_playlist;

	uint64_t m_hopo_frequency = 0;
	uint64_t m_sustain_cutoff_threshold = 0;
	uint16_t m_hopofreq_old = UINT16_MAX;
	bool m_eighthnote_hopo = false;
	unsigned char m_multiplier_note = 116;
	DrumType_Enum m_baseDrumType = DrumType_Enum::LEGACY;

	std::vector<Modifiers::Modifier> m_modifiers;

	uint32_t m_tickrate = 192;
	SyncTrack m_sync;
	SongEvents m_events;

	struct
	{
		InstrumentalTrack<GuitarNote<5>>      lead_5;
		InstrumentalTrack<GuitarNote<6>>      lead_6;
		InstrumentalTrack<GuitarNote<5>>      bass_5;
		InstrumentalTrack<GuitarNote<6>>      bass_6;
		InstrumentalTrack<GuitarNote<5>>      rhythm;
		InstrumentalTrack<GuitarNote<5>>      coop;
		InstrumentalTrack<Keys<5>>            keys;
		InstrumentalTrack<DrumNote<DrumPad_Pro, 4>>  drums4_pro;
		InstrumentalTrack<DrumNote<DrumPad, 5>> drums5;
		VocalTrack<1>                         vocals;
		VocalTrack<3>                         harmonies;
	} m_noteTracks;

	std::u32string m_midiSequenceName;

	std::u32string m_musicStream;
	std::u32string m_guitarStream;
	std::u32string m_rhythmStream;
	std::u32string m_bassStream;
	std::u32string m_keysStream;
	std::u32string m_drumStream;
	std::u32string m_drum2Stream;
	std::u32string m_drum3Stream;
	std::u32string m_drum4Stream;
	std::u32string m_vocalStream;
	std::u32string m_harmonyStream;
	std::u32string m_crowdStream;
};
