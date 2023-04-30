#pragma once
#include "Entry/LibraryEntry.h"
class SongLibrary
{
public:
	void scan(const std::vector<std::filesystem::path>& baseDirectories);

private:
	void scanDirectory(const std::filesystem::path& directory);
};

