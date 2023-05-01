#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "LibraryCategory.h"

class SongLibrary
{
	static constexpr uint32_t s_CACHE_VERSION = 0x05012023;

public:
	void scan(const std::vector<std::filesystem::path>& baseDirectories);
	void finalize();
	void writeToCacheFile() const;
	void clear();

	[[nodiscard]] size_t getNumSongs() const noexcept;

private:
	void scanDirectory(const std::filesystem::path& directory);
	void addEntry(LibraryEntry&& entry, LibraryEntry::ChartType type, const std::filesystem::path& chartPath);

private:
	SimpleFlatMap<std::vector<LibraryEntry>, MD5> m_songlist;

	LibraryCategory<SongAttribute::TITLE>       m_category_title;
	LibraryCategory<SongAttribute::ARTIST>      m_category_artist;
	LibraryCategory<SongAttribute::ALBUM>       m_category_album;
	LibraryCategory<SongAttribute::GENRE>       m_category_genre;
	LibraryCategory<SongAttribute::YEAR>        m_category_year;
	LibraryCategory<SongAttribute::CHARTER>     m_category_charter;
	LibraryCategory<SongAttribute::PLAYLIST>    m_category_playlist;
	LibraryCategory<SongAttribute::ARTIST, LibraryCategory<SongAttribute::ALBUM>> m_category_artistAlbum;
};

