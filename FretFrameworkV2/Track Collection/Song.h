#pragma once
#include "Tracks/Instrumental/InstrumentalTrack_DrumsLegacy.h"
#include "Tracks/Vocal/VocalTrack.h"
#include "../Notes/GuitarNote.h"
#include "../Notes/Keys.h"
#include "MicrosPerQuarter.h"

class Song
{
	struct
	{
		InstrumentalTrack<GuitarNote<5>>            lead_5;
		InstrumentalTrack<GuitarNote<6>>            lead_6;
		InstrumentalTrack<GuitarNote<5>>            bass_5;
		InstrumentalTrack<GuitarNote<6>>            bass_6;
		InstrumentalTrack<GuitarNote<5>>            rhythm;
		InstrumentalTrack<GuitarNote<5>>            coop;
		InstrumentalTrack<Keys<5>>                  keys;
		InstrumentalTrack<DrumNote<4, true>>        drums4_pro;
		InstrumentalTrack<DrumNote<5, false>>       drums5;
		VocalTrack<1>                               vocals;
		VocalTrack<3>                               harmonies;

		Track* const array[11] =
		{
			&lead_5,
			&lead_6,
			&bass_5,
			&bass_6,
			&rhythm,
			&coop,
			&keys,
			&drums4_pro,
			&drums5,
			&vocals,
			&harmonies
		};
	} m_noteTracks;

	uint32_t m_tickrate = 192;
	SimpleFlatMap<MicrosPerQuarter> m_tempoMarkers;
	SimpleFlatMap<TimeSig> m_timeSigs;
	SimpleFlatMap<UnicodeString> m_sectionMarkers;
	SimpleFlatMap<std::vector<std::u32string>> m_globalEvents;

	std::u32string m_midiSequenceName;

public:
	bool load(const std::filesystem::path& path) noexcept;
	bool save(std::filesystem::path path) noexcept;
	void resetTempoMap();
	void clear();

private:
	static const std::filesystem::path s_EXTS_CHT[2];
	static const std::filesystem::path s_EXTS_MID[2];
	static const std::filesystem::path s_EXT_BCH;

	void load_cht(const std::filesystem::path& path);
	void load_bch(const std::filesystem::path& path);
	void load_mid(const std::filesystem::path& path);

	bool load_tempoMap(CommonChartParser* parser);
	bool load_events(CommonChartParser* parser);
	bool load_noteTrack(CommonChartParser* parser);

	int  load_songInfo_cht(TxtFileReader& reader);
	bool load_events_V1(TxtFileReader& reader);
	bool load_noteTrack_V1(TxtFileReader& reader, InstrumentalTrack<DrumNote_Legacy>& drumsLegacy);

	bool addSection_midi(uint32_t position, std::string_view str);

	void save_cht(const std::filesystem::path& path);
	void save_bch(const std::filesystem::path& path);
	void save_mid(const std::filesystem::path& path);

	void save(CommonChartWriter* writer) const;
	void save_header(CommonChartWriter* writer) const;
	void save_tempoMap(CommonChartWriter* writer) const;
	void save_events(CommonChartWriter* writer) const;
	void save_noteTracks(CommonChartWriter* writer) const;
};
