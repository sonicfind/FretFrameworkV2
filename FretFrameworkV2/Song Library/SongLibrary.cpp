#include "SongLibrary.h"
#include <iostream>

void SongLibrary::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	for (const auto& directory : baseDirectories)
		scanDirectory(directory);
}

void SongLibrary::finalize()
{
	for (auto& node : m_songlist)
		for (auto& entry : *node)
		{
			entry.finalize();
			m_category_title.add(entry);
			m_category_artist.add(entry);
			m_category_genre.add(entry);
			m_category_year.add(entry);
			m_category_charter.add(entry);

			m_category_album.add(entry);
			m_category_artistAlbum.add(entry);

			m_category_playlist.add(entry);
		}
}

void SongLibrary::clear()
{
	m_songlist.clear();
}

size_t SongLibrary::getNumSongs() const noexcept
{
	size_t size = 0;
	for (const auto& node : m_songlist)
		size += node->size();
	return size;
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
			if (ini)
				addEntry(LibraryEntry(*charts[i], *ini), CHARTTYPES[i].second, charts[i]->path());
			else
				addEntry(LibraryEntry(*charts[i]), CHARTTYPES[i].second, charts[i]->path());
			return;
		}
	}

	for (const auto& subDirectory : directories)
		scanDirectory(subDirectory);
}

void SongLibrary::addEntry(LibraryEntry&& entry, LibraryEntry::ChartType type, const std::filesystem::path& chartPath)
{
	LoadedFile file(chartPath);
	if (entry.scan(file, type))
		m_songlist[file.calculateMD5()].push_back(std::move(entry));
	else
		std::cout << "Failed: " << UnicodeString::U32ToStr(chartPath.u32string()) << '\n';
}
