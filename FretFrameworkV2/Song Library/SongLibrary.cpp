#include "SongLibrary.h"

void SongLibrary::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	for (const auto& directory : baseDirectories)
		scanDirectory(directory);
}

void SongLibrary::scanDirectory(const std::filesystem::path& directory)
{
	std::vector<std::filesystem::path> directories;
	for (const auto& file : std::filesystem::directory_iterator(directory))
		if (file.is_directory())
			directories.push_back(file.path());

	for (const auto& subDirectory : directories)
		scanDirectory(subDirectory);
}
