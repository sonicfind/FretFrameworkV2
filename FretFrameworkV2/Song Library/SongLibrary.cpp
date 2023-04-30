#include "SongLibrary.h"
#include <iostream>

void SongLibrary::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	for (const auto& directory : baseDirectories)
		scanDirectory(directory);
}

void SongLibrary::scanDirectory(const std::filesystem::path& directory)
{
	static const std::pair<std::filesystem::path, LibraryEntry::ChartType> CHARTTYPES[] =
	{
		{ U"notes.bch",	  LibraryEntry::BCH },
		{ U"notes.cht",   LibraryEntry::CHT },
		{ U"notes.mid",   LibraryEntry::MID },
		{ U"notes.midi",  LibraryEntry::MID },
		{ U"notes.chart", LibraryEntry::CHT },
	};
	static const std::filesystem::path ININAME = U"song.ini";

	std::optional<std::filesystem::directory_entry> charts[5]{};
	std::optional<std::filesystem::directory_entry> ini;
	std::vector<std::filesystem::path> directories;
	for (const auto& file : std::filesystem::directory_iterator(directory))
	{
		const std::filesystem::path& path = file.path();
		if (file.is_directory())
		{
			directories.push_back(path);
			continue;
		}

		const std::filesystem::path filename = path.filename();
		if (filename == ININAME)
		{
			ini = file;
			continue;
		}

		for (size_t i = 0; i < std::size(CHARTTYPES); ++i)
		{
			if (filename == CHARTTYPES[i].first)
			{
				charts[i] = file;
				break;
			}
		}
	}

	if (!ini)
	{
		charts[0].reset();
		charts[2].reset();
		charts[3].reset();
	}

	for (size_t i = 0; i < std::size(CHARTTYPES); ++i)
	{
		if (charts[i])
		{
			LibraryEntry entry(*charts[i]);
			if (ini)
				entry.readIni(*ini);

			LoadedFile file(charts[i]->path());
			entry.scan(file, CHARTTYPES[i].second);
			return;
		}
	}

	for (const auto& subDirectory : directories)
		scanDirectory(subDirectory);
}
