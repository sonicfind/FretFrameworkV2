#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Entry/LibraryEntry.h"

class SongLibrary
{
public:
	void scan(const std::vector<std::filesystem::path>& baseDirectories);
	void clear();

	[[nodiscard]] size_t getNumSongs() const noexcept;

private:
	void scanDirectory(const std::filesystem::path& directory);
	void addEntry(LibraryEntry&& entry, LibraryEntry::ChartType type, const std::filesystem::path& chartPath);

private:
	SimpleFlatMap<std::vector<LibraryEntry>, MD5> m_songlist;
};

