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
		if (file.is_directory())
		{
			directories.push_back(file.path());
			continue;
		}

		const std::filesystem::path filename = file.path().filename();
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
			if (entry.scan(file, CHARTTYPES[i].second))
				addEntry(file.calculateMD5(), std::move(entry));
			else
				std::cout << "Failed: " << UnicodeString::U32ToStr(charts[i]->path().u32string()) << '\n';
			return;
		}
	}

	for (const auto& subDirectory : directories)
		scanDirectory(subDirectory);
}

void SongLibrary::addEntry(MD5 hash, LibraryEntry&& entry)
{
	m_songlist[hash].push_back(std::move(entry));
}

void SongLibrary::readFromCacheFile()
{
	if (!std::filesystem::exists("songcache.bin"))
		return;

	BufferedBinaryReader reader("songcache.bin");
	if (s_CACHE_VERSION != reader.extract<uint32_t, false>())
		return;

	const auto getStrings = [&reader]()
	{
		reader.setNextSectionBounds();

		const uint32_t numStrings = reader.extract<uint32_t>();
		std::vector<UnicodeString> strings;
		strings.reserve(numStrings);
		for (uint32_t i = 0; i < numStrings; ++i)
			strings.emplace_back(reader.extractString());
		return strings;
	};

	m_stringBuffers.titles = getStrings();
	m_stringBuffers.artists = getStrings();
	m_stringBuffers.albums = getStrings();
	m_stringBuffers.genres = getStrings();
	m_stringBuffers.years = getStrings();
	m_stringBuffers.charters = getStrings();
	m_stringBuffers.playlists = getStrings();
}

void SongLibrary::writeToCacheFile() const
{
	std::unordered_map<const LibraryEntry*, std::pair<MD5, CacheIndices>> nodes;
	for (auto& node : m_songlist)
	{
		const std::pair<MD5, CacheIndices> base = { node.key, {} };
		for (auto& entry : *node)
			nodes.insert({ &entry, base });
	}

	BufferedBinaryWriter writer("songcache.bin");
	writer.write(s_CACHE_VERSION);
	m_category_title.mapToCache(writer, nodes);
	m_category_artist.mapToCache(writer, nodes);
	m_category_album.mapToCache(writer, nodes);
	m_category_genre.mapToCache(writer, nodes);
	m_category_year.mapToCache(writer, nodes);
	m_category_charter.mapToCache(writer, nodes);
	m_category_playlist.mapToCache(writer, nodes);
	
	writer.write((uint32_t)nodes.size());
	for (const auto& node : nodes)
	{
		node.first->serializeChartData(writer);
		writer.append(node.second.first);
		node.first->serializeSongInfo(writer);
		writer.append(node.second.second);
		writer.writeBuffer();
	}
}
