#pragma once
#include "InstrumentalScan.h"
#include "VocalScan.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Notes/DrumNote.h"
#include "Types/UnicodeString.h"

class CacheEntry
{
	static const UnicodeString s_DEFAULT_NAME;
	static const UnicodeString s_DEFAULT_ARTIST;
	static const UnicodeString s_DEFAULT_ALBUM;
	static const UnicodeString s_DEFAULT_GENRE;
	static const UnicodeString s_DEFAULT_YEAR;
	static const UnicodeString s_DEFAULT_CHARTER;

	struct
	{
		InstrumentalScan_Extended<GuitarNote<5>>      lead_5;
		InstrumentalScan_Extended<GuitarNote<6>>      lead_6;
		InstrumentalScan_Extended<GuitarNote<5>>      bass_5;
		InstrumentalScan_Extended<GuitarNote<6>>      bass_6;
		InstrumentalScan_Extended<GuitarNote<5>>      rhythm;
		InstrumentalScan_Extended<GuitarNote<5>>      coop;
		InstrumentalScan_Extended<Keys<5>>            keys;
		InstrumentalScan_Extended<DrumNote<4, true>>  drums4_pro;
		InstrumentalScan_Extended<DrumNote<5, false>> drums5;
		VocalScan<1>                                  vocals;
		VocalScan<3>                                  harmonies;
	} m_noteTracks;

	std::vector<Modifiers::Modifier> m_modifiers;

	std::filesystem::path m_directory;
	std::filesystem::path m_filename;

	std::filesystem::file_time_type m_chartModifiedTime;
	std::filesystem::file_time_type m_iniModifiedTime;

public:
	CacheEntry(std::filesystem::file_time_type chartTime);
	void readIni(const std::filesystem::path& path, std::filesystem::file_time_type iniTime);
	bool scan(const std::filesystem::path& path) noexcept;
private:
	std::vector<Modifiers::Modifier>::const_iterator getModifier(std::string_view name) const noexcept;
	std::vector<Modifiers::Modifier>::iterator getModifier(std::string_view name) noexcept;

private:
	static const std::filesystem::path s_EXTS_CHT[2];
	static const std::filesystem::path s_EXTS_MID[2];
	static const std::filesystem::path s_EXT_BCH;

	bool scan_cht(const std::filesystem::path& path);
	void scan_bch(const std::filesystem::path& path);
	void scan_mid(const std::filesystem::path& path);

	void scan(CommonChartParser& parser);
	void scan_noteTrack(CommonChartParser& parser);

	void scan_cht_V1(ChtFileReader& reader);

	void reorderModifiers();
	void writeIni();
};
