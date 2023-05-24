#pragma once
#include "Tracks/Instrumental/ProGuitarTrack.h"
#include "Tracks/Instrumental/ProKeysDifficulty.h"
#include "Tracks/Vocal/VocalTrack.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Notes/DrumNote.h"
#include "ChartSpecifiers.h"
#include "SyncTrack.h"
#include "SongEvents.h"
#include "Serialization/ChtFileReader.h"

class Song
{
public:
	Song() = default;
	Song(const std::filesystem::path& directory);
	Song(const std::filesystem::path& directory,
		 const std::u32string& name,
	     const std::u32string& artist,
	     const std::u32string& album,
	     const std::u32string& genre,
	     const std::u32string& year,
	     const std::u32string& charter,
	     const std::u32string& playlist,
	     const uint64_t hopoFrequency,
	     const uint16_t hopofreq_old,
	     const unsigned char multiplierNote,
	     const bool eighthNoteHopo,
	     const uint64_t sustainCutoffThreshold,
	     const DrumType_Enum drumType);

	void loadIni();
	bool compareToIni();
	bool load(const std::filesystem::path& chartFile, const ChartType type, bool fullLoad) noexcept;
	bool save(ChartType type) const noexcept;
	void clear();

private:
	void setSustainThreshold() const;
	void checkStartOfTempoMap();
	
	void saveIni() const;
	void validateAudioStreams();

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
		InstrumentalTrack<GuitarNote<5>>            lead_5;
		InstrumentalTrack<GuitarNote<6>>            lead_6;
		InstrumentalTrack<GuitarNote<5>>            bass_5;
		InstrumentalTrack<GuitarNote<6>>            bass_6;
		InstrumentalTrack<GuitarNote<5>>            rhythm;
		InstrumentalTrack<GuitarNote<5>>            coop;
		InstrumentalTrack<Keys<5>>                  keys;
		InstrumentalTrack<DrumNote<DrumPad_Pro, 4>> drums4_pro;
		InstrumentalTrack<DrumNote<DrumPad, 5>>     drums5;
		InstrumentalTrack<GuitarNote_Pro<6, 17>>    proGuitar_17;
		InstrumentalTrack<GuitarNote_Pro<6, 22>>    proGuitar_22;
		InstrumentalTrack<GuitarNote_Pro<4, 17>>    proBass_17;
		InstrumentalTrack<GuitarNote_Pro<4, 22>>    proBass_22;
		InstrumentalTrack<Keys_Pro>                 proKeys;
		VocalTrack<1>                               vocals;
		VocalTrack<3>                               harmonies;
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
