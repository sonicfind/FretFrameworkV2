#pragma once
#include "InstrumentalScan.h"
#include "VocalScan.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Notes/DrumNote.h"
#include "Types/UnicodeString.h"

class LibraryEntry
{
public:
	enum ChartType
	{
		BCH,
		CHT,
		MID
	};

public:
	LibraryEntry(const std::filesystem::directory_entry& chartFile);
	LibraryEntry(const std::filesystem::directory_entry& chartFile, const std::filesystem::directory_entry& iniFile);
	bool scan(const LoadedFile& file, const ChartType type) noexcept;
	void finalize();

	const UnicodeString& getArtist() const { return *m_artist; }
	const UnicodeString& getName() const { return *m_name; }
	const UnicodeString& getAlbum() const { return *m_album; }
	const UnicodeString& getGenre() const { return *m_genre; }
	const UnicodeString& getYear() const { return *m_year; }
	const UnicodeString& getCharter() const { return *m_charter; }
	const UnicodeString& getPlaylist() const { return *m_playlist; }
	const uint32_t& getSongLength() const { return m_song_length; }
	const std::filesystem::path& getDirectory() const { return m_chartFile.path().parent_path(); }

private:
	class OptionalModifier
	{
		Modifiers::Modifier* mod = nullptr;

	public:
		OptionalModifier() = default;
		OptionalModifier(Modifiers::Modifier& md) : mod(&md){}
		Modifiers::Modifier* operator->() { return mod; }
		const Modifiers::Modifier* operator->() const noexcept { return mod; }
		Modifiers::Modifier& operator*() { return *mod; }
		const Modifiers::Modifier& operator*() const noexcept { return *mod; }
		operator bool() { return mod != nullptr; }
	};

	struct OptionalModifier_const
	{
		const Modifiers::Modifier* mod = nullptr;

	public:
		OptionalModifier_const() = default;
		OptionalModifier_const(const Modifiers::Modifier& md) : mod(&md) {}
		const Modifiers::Modifier* operator->() const noexcept { return mod; }
		const Modifiers::Modifier& operator*() const noexcept { return *mod; }
		operator bool() { return mod != nullptr; }
	};
	OptionalModifier_const getModifier(std::string_view name) const noexcept;
	OptionalModifier getModifier(std::string_view name) noexcept;

private:
	void scan_cht(const LoadedFile& file);
	void scan_bch(const LoadedFile& file);
	void scan_mid(const LoadedFile& file);

	void scan(CommonChartParser& parser);
	void scan_noteTrack(CommonChartParser& parser);

	int scan_header_cht(ChtFileReader& reader);
	void scan_cht_V1(ChtFileReader& reader);

	bool validateForNotes() const noexcept;
	void reorderModifiers();
	void mapModifierVariables();
	void writeIni();

private:
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
	} m_scanTracks;

	const UnicodeString* m_name = nullptr;
	const UnicodeString* m_artist = nullptr;
	const UnicodeString* m_album = nullptr;
	const UnicodeString* m_genre = nullptr;
	const UnicodeString* m_year = nullptr;
	const UnicodeString* m_charter = nullptr;
	const UnicodeString* m_playlist = nullptr;
	uint32_t m_song_length = 0;

	float m_previewRange[2]{};
	uint16_t m_album_track = UINT16_MAX;
	uint16_t m_playlist_track = UINT16_MAX;
	std::u32string m_icon;
	std::u32string m_source;
	uint32_t m_hopo_frequency = 0;

	std::vector<Modifiers::Modifier> m_modifiers;

	std::filesystem::directory_entry m_chartFile;
	std::filesystem::file_time_type m_iniModifiedTime;

	bool m_rewriteIni = false;
};
