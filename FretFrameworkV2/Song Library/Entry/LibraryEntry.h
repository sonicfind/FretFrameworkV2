#pragma once
#include "Song/ChartType.h"
#include "DrumScan.h"
#include "VocalScan.h"
#include "Notes/GuitarNote.h"
#include "Notes/Keys.h"
#include "Types/UnicodeString.h"
#include "PtrWrapper/PtrWrapper.h"
#include "Serialization/ChtFileReader.h"
#include "Serialization/BufferedBinaryReader.h"
#include "Serialization/BufferedBinaryWriter.h"

enum class SongAttribute
{
	UNSPECIFIED,
	TITLE,
	ARTIST,
	ALBUM,
	GENRE,
	YEAR,
	CHARTER,
	PLAYLIST,
	SONG_LENGTH,
	SOURCE,
};

class LibraryEntry
{
public:
	using UnicodeWrapper = PointerWrapper<const UnicodeString>;

public:
	LibraryEntry(const std::pair<std::filesystem::path, ChartType>& chartpath, const std::filesystem::file_time_type& chartLastWrite, const std::filesystem::file_time_type& iniLastWrite = {});

	void mapStrings(UnicodeWrapper name, UnicodeWrapper artist, UnicodeWrapper album, UnicodeWrapper genre, UnicodeWrapper year, UnicodeWrapper charter, UnicodeWrapper playlist);
	void readIni(const std::filesystem::path& iniPath, const std::filesystem::file_time_type& iniLastWrite);
	bool scan(const LoadedFile& file) noexcept;
	void finalize();
	void extractSongInfo(BufferedBinaryReader& reader);
	void serializeFileInfo(BufferedBinaryWriter& writer) const noexcept;
	void serializeSongInfo(BufferedBinaryWriter& writer) const noexcept;

	const UnicodeString& getArtist() const noexcept { return *m_artist; }
	const UnicodeString& getName() const noexcept { return *m_name; }
	const UnicodeString& getAlbum() const noexcept { return *m_album; }
	const UnicodeString& getGenre() const noexcept { return *m_genre; }
	const UnicodeString& getYear() const noexcept { return *m_year; }
	const UnicodeString& getCharter() const noexcept { return *m_charter; }
	const UnicodeString& getPlaylist() const noexcept { return *m_playlist; }
	uint64_t getSongLength() const noexcept { return m_song_length; }
	uint32_t getHopofreq_Old() const noexcept { return m_hopofreq_Old; }
	uint64_t getHopoFrequency() const noexcept { return m_hopo_frequency; }
	uint32_t getMultiplierNote() const noexcept { return m_multiplier_note; }
	uint64_t getSustainCutoffThreshold() const noexcept { return m_sustain_cutoff_threshold; }
	bool getEightNoteHopo() const noexcept { return m_eighthnote_hopo; }

	std::filesystem::path getDirectory() const noexcept { return m_chartFile.first.parent_path(); }

	std::filesystem::directory_entry getFileEntry() const noexcept { return std::filesystem::directory_entry(m_chartFile.first); }
	ChartType getChartType() const noexcept { return m_chartFile.second; }
	const auto& getLastWriteTime() const noexcept { return m_chartWriteTime; }

	template<SongAttribute Attribute>
	const auto& getAttribute() const noexcept
	{
		if constexpr (Attribute == SongAttribute::TITLE)
			return *m_name;
		else if constexpr (Attribute == SongAttribute::ARTIST)
			return *m_artist;
		else if constexpr (Attribute == SongAttribute::ALBUM)
			return *m_album;
		else if constexpr (Attribute == SongAttribute::GENRE)
			return *m_genre;
		else if constexpr (Attribute == SongAttribute::YEAR)
			return *m_year;
		else if constexpr (Attribute == SongAttribute::CHARTER)
			return *m_charter;
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
			return *m_playlist;
		else if constexpr (Attribute == SongAttribute::SONG_LENGTH)
			return m_song_length;
		else if constexpr (Attribute == SongAttribute::SOURCE)
			return m_source;
	}

	template <SongAttribute Attribute>
	bool isBelow(const LibraryEntry& other) const
	{
		if constexpr (Attribute == SongAttribute::ALBUM)
		{
			if (m_album_track != other.m_album_track)
				return m_album_track < other.m_album_track;
		}
		else if constexpr (Attribute == SongAttribute::PLAYLIST)
		{
			if (m_playlist_track != other.m_playlist_track)
				return m_playlist_track < other.m_playlist_track;
		}

		int strCmp = 0;
		if ((strCmp = m_name->compare(*other.m_name)) != 0 ||
			(strCmp = m_artist->compare(*other.m_artist)) != 0 ||
			(strCmp = m_album->compare(*other.m_album)) != 0 ||
			(strCmp = m_charter->compare(*other.m_charter)) != 0)
			return strCmp < 0;
		else
			return getDirectory() < other.getDirectory();
	}

	std::u32string grabLoadingPhrase() const noexcept;

	DrumType_Enum getDrumType() const noexcept;
	PointerWrapper<const Modifiers::Modifier> getModifier(std::string_view name) const noexcept;
private:
	PointerWrapper<Modifiers::Modifier> getModifier(std::string_view name) noexcept;
	DrumType_Enum getDrumTypeFromModifier() const noexcept;

private:
	void scan_cht(const LoadedFile& file);
	void scan_bch(const LoadedFile& file);
	void scan_mid(const LoadedFile& file);

	void traverse(CommonChartParser& parser);

	int scan_header_cht(ChtFileReader& reader);
	void traverse_cht_V1(ChtFileReader& reader);

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
		InstrumentScan<GuitarNote<5>> lead_5;
		InstrumentScan<GuitarNote<6>> lead_6;
		InstrumentScan<GuitarNote<5>> bass_5;
		InstrumentScan<GuitarNote<6>> bass_6;
		InstrumentScan<GuitarNote<5>> rhythm;
		InstrumentScan<GuitarNote<5>> coop;
		InstrumentScan<Keys<5>> keys;
		InstrumentScan<DrumNote<DrumPad, 4>> drums4;
		InstrumentScan<DrumNote<DrumPad_Pro, 4>> drums4_pro;
		InstrumentScan<DrumNote<DrumPad, 5>> drums5;
		VocalScan<1> vocals;
		VocalScan<3> harmonies;
	} m_scanTracks;

	UnicodeWrapper m_name;
	UnicodeWrapper m_artist;
	UnicodeWrapper m_album ;
	UnicodeWrapper m_genre;
	UnicodeWrapper m_year;
	UnicodeWrapper m_charter;
	UnicodeWrapper m_playlist;

	uint64_t m_song_length = 0;
	float m_previewRange[2]{};
	uint16_t m_album_track = UINT16_MAX;
	uint16_t m_playlist_track = UINT16_MAX;
	std::u32string m_icon;
	std::u32string m_source;
	
	uint64_t m_hopo_frequency = 0;
	uint64_t m_sustain_cutoff_threshold = 0;
	uint16_t m_hopofreq_Old = UINT16_MAX;
	bool m_eighthnote_hopo = false;
	unsigned char m_multiplier_note = 116;

	UnicodeString m_directory_playlist;

	std::vector<Modifiers::Modifier> m_modifiers;

	std::pair<std::filesystem::path, ChartType> m_chartFile;
	std::filesystem::file_time_type m_chartWriteTime;
	std::filesystem::file_time_type m_iniWriteTime;

	bool m_rewriteIni = false;
};
