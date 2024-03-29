#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include <unordered_set>
#include "LibraryCategory.h"
#include <mutex>

class SongLibrary
{
	static constexpr uint32_t s_CACHE_VERSION = 0x05252301;

public:
	void runFullScan(const std::vector<std::u32string>& baseDirectories);
	bool runPartialScan();
	void clear();

	[[nodiscard]] size_t getNumSongs() const noexcept;

private:
	void finalize();
	std::optional<BufferedBinaryReader> loadCachefile();
	void readStrings(BufferedBinaryReader& reader);
	void readNodes(BufferedBinaryReader& reader, auto&& validationFunc);
	void writeToCacheFile() const;

	void scanDirectory(const std::filesystem::path& directory);
	void addEntry(MD5 hash, LibraryEntry&& entry);
	void markScannedDirectory(const std::filesystem::path& directory);
	bool findOrMarkDirectory(const std::filesystem::path& directory);

private:
	SimpleFlatMap<std::vector<LibraryEntry>, MD5> m_songlist;

	struct
	{
		std::vector<UnicodeString> titles;
		std::vector<UnicodeString> artists;
		std::vector<UnicodeString> albums;
		std::vector<UnicodeString> genres;
		std::vector<UnicodeString> years;
		std::vector<UnicodeString> charters;
		std::vector<UnicodeString> playlists;
	} m_stringBuffers;

	struct
	{
		LibraryCategory<SongAttribute::TITLE>    title;
		LibraryCategory<SongAttribute::ARTIST>   artist;
		LibraryCategory<SongAttribute::ALBUM>    album;
		LibraryCategory<SongAttribute::GENRE>    genre;
		LibraryCategory<SongAttribute::YEAR>     year;
		LibraryCategory<SongAttribute::CHARTER>  charter;
		LibraryCategory<SongAttribute::PLAYLIST> playlist;
		LibraryCategory<SongAttribute::ARTIST, LibraryCategory<SongAttribute::ALBUM>> artistAlbum;
	} m_categories;

	std::unordered_set<std::filesystem::path> m_preScannedDirectories;
	std::mutex m_directoryMutex;
	std::mutex m_entryMutex;
};

