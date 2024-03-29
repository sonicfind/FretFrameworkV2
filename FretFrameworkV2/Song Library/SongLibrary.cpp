#include "SongLibrary.h"
#include "TaskQueue/TaskQueue.h"
#include <iostream>
const std::filesystem::path ININAME = U"song.ini";

const std::pair<std::filesystem::path, ChartType> CHARTTYPES[] =
{
	{ U"notes.bch",	  ChartType::BCH },
	{ U"notes.cht",   ChartType::CHT },
	{ U"notes.mid",   ChartType::MID },
	{ U"notes.midi",  ChartType::MID },
	{ U"notes.chart", ChartType::CHT },
};

void SongLibrary::runFullScan(const std::vector<std::u32string>& baseDirectories)
{
	if (auto reader = loadCachefile())
	{
		const auto validate = [&baseDirectories](BufferedBinaryReader& reader) -> std::optional<LibraryEntry>
		{
			const std::u32string directory = UnicodeString::strToU32(reader.extractString());
			const auto checkBase = [&baseDirectories, &directory]
			{
				for (const auto& base : baseDirectories)
					if (directory.starts_with(base))
						return true;
				return false;
			};

			if (!checkBase())
				return {};

			const std::filesystem::path filename = UnicodeString::strToU32(reader.extractString());
			const std::filesystem::directory_entry chartFile(directory / filename);
			const std::filesystem::directory_entry iniFile(directory / ININAME);

			if (!chartFile.exists() || !iniFile.exists())
				return {};

			const std::filesystem::file_time_type chartWriteTime = (std::filesystem::file_time_type)reader.extract<std::filesystem::file_time_type::duration>();
			const std::filesystem::file_time_type iniWriteTime = (std::filesystem::file_time_type)reader.extract<std::filesystem::file_time_type::duration>();
			if (chartFile.last_write_time() != chartWriteTime || iniFile.last_write_time() != iniWriteTime)
				return {};

			for (size_t i = 0; i < std::size(CHARTTYPES); ++i)
				if (filename == CHARTTYPES[i].first)
					return LibraryEntry({ directory / filename, CHARTTYPES[i].second }, chartWriteTime, iniWriteTime);
			return {};
		};

		readStrings(*reader);
		readNodes(*reader, validate);
	}

	auto& tasks = TaskQueue::getInstance();
	for (const auto& directory : baseDirectories)
		tasks.addTask([this, &directory] { scanDirectory(directory); });

	tasks.waitForCompletedTasks();
	finalize();
	writeToCacheFile();
}

bool SongLibrary::runPartialScan()
{
	if (auto reader = loadCachefile())
	{
		static constexpr auto validate = [](BufferedBinaryReader& reader)  -> std::optional<LibraryEntry>
		{
			const std::filesystem::path directory = UnicodeString::strToU32(reader.extractString());
			const std::filesystem::path filename = UnicodeString::strToU32(reader.extractString());

			const std::filesystem::file_time_type chartWriteTime = (std::filesystem::file_time_type)reader.extract<std::filesystem::file_time_type::duration>();
			const std::filesystem::file_time_type iniWriteTime = (std::filesystem::file_time_type)reader.extract<std::filesystem::file_time_type::duration>();

			for (size_t i = 0; i < std::size(CHARTTYPES); ++i)
				if (filename == CHARTTYPES[i].first)
					return LibraryEntry({ directory / filename, CHARTTYPES[i].second }, chartWriteTime, iniWriteTime);
			return {};
		};

		readStrings(*reader);
		readNodes(*reader, validate);
		finalize();
		return true;
	}
	return false;
}

void SongLibrary::clear()
{
	m_stringBuffers.titles.clear();
	m_stringBuffers.artists.clear();
	m_stringBuffers.albums.clear();
	m_stringBuffers.genres.clear();
	m_stringBuffers.years.clear();
	m_stringBuffers.charters.clear();
	m_stringBuffers.playlists.clear();
	m_categories.title.clear();
	m_categories.artist.clear();
	m_categories.album.clear();
	m_categories.genre.clear();
	m_categories.year.clear();
	m_categories.charter.clear();
	m_categories.playlist.clear();
	m_categories.artistAlbum.clear();
	m_songlist.clear();
}

size_t SongLibrary::getNumSongs() const noexcept
{
	size_t size = 0;
	for (const auto& node : m_songlist)
		size += node->size();
	return size;
}

std::optional<BufferedBinaryReader> SongLibrary::loadCachefile()
{
	if (std::filesystem::exists("songcache.bin"))
	{
		try
		{
			BufferedBinaryReader reader("songcache.bin");
			if (s_CACHE_VERSION == reader.extract<uint32_t>())
				return std::move(reader);
		}
		catch (...) {}
	}
	return {};
}

void SongLibrary::readStrings(BufferedBinaryReader& reader)
{
	static constexpr auto getStrings = [](BufferedBinaryReader& reader)
	{
		reader.enterSection();

		const uint32_t numStrings = reader.extract<uint32_t>();
		std::vector<UnicodeString> strings;
		strings.reserve(numStrings);
		for (uint32_t i = 0; i < numStrings; ++i)
			strings.emplace_back(reader.extractString());

		reader.exitSection();
		return strings;
	};

	m_stringBuffers.titles = getStrings(reader);
	m_stringBuffers.artists = getStrings(reader);
	m_stringBuffers.albums = getStrings(reader);
	m_stringBuffers.genres = getStrings(reader);
	m_stringBuffers.years = getStrings(reader);
	m_stringBuffers.charters = getStrings(reader);
	m_stringBuffers.playlists = getStrings(reader);
}

void SongLibrary::readNodes(BufferedBinaryReader& reader, auto&& validationFunc)
{
	auto& tasks = TaskQueue::getInstance();
	const uint32_t numNodes = reader.extract<uint32_t>();
	for (uint32_t i = 0; i < numNodes; ++i)
	{
		reader.enterSection();
		tasks.addTask([this, reader, &validationFunc]() mutable {
			if (std::optional<LibraryEntry> entry = validationFunc(reader))
			{
				const UnicodeString& name = m_stringBuffers.titles[reader.extract<uint32_t>()];
				const UnicodeString& artist = m_stringBuffers.artists[reader.extract<uint32_t>()];
				const UnicodeString& album = m_stringBuffers.albums[reader.extract<uint32_t>()];
				const UnicodeString& genre = m_stringBuffers.genres[reader.extract<uint32_t>()];
				const UnicodeString& year = m_stringBuffers.years[reader.extract<uint32_t>()];
				const UnicodeString& charter = m_stringBuffers.charters[reader.extract<uint32_t>()];
				const UnicodeString& playlist = m_stringBuffers.playlists[reader.extract<uint32_t>()];
				entry->mapStrings(name, artist, album, genre, year, charter, playlist);
				entry->extractSongInfo(reader);

				markScannedDirectory(entry->getDirectory());
				addEntry(reader.extract<MD5>(), std::move(*entry));
			}
			});
		reader.exitSection();
	}

	tasks.waitForCompletedTasks();
}

void SongLibrary::scanDirectory(const std::filesystem::path& directory)
{
	if (findOrMarkDirectory(directory))
		return;

	std::optional<std::filesystem::directory_entry> charts[5]{};
	std::optional<std::filesystem::directory_entry> ini;
	std::vector<std::filesystem::path> directories;
	try
	{
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
	}
	catch (...)
	{
		return;
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
			try
			{
				LibraryEntry entry({ charts[i]->path(), CHARTTYPES[i].second }, charts[i]->last_write_time());
				if (ini)
					entry.readIni(ini->path(), ini->last_write_time());

				LoadedFile file(charts[i]->path());
				if (entry.scan(file))
					addEntry(file.calculateMD5(), std::move(entry));
			}
			catch (std::runtime_error err)
			{

			}
			return;
		}
	}

	auto& tasks = TaskQueue::getInstance();
	for (auto& subDirectory : directories)
		tasks.addTask([this, dir = std::move(subDirectory)] { scanDirectory(dir); });
		
}

void SongLibrary::finalize()
{
	m_preScannedDirectories.clear();
	for (auto& node : m_songlist)
	{
		for (auto& entry : *node)
		{
			entry.finalize();
			m_categories.title.add(entry);
			m_categories.artist.add(entry);
			m_categories.album.add(entry);
			m_categories.genre.add(entry);
			m_categories.year.add(entry);
			m_categories.charter.add(entry);
			m_categories.artistAlbum.add(entry);
			m_categories.playlist.add(entry);
		}
	}
}

void SongLibrary::writeToCacheFile() const
{
	std::unordered_map<const LibraryEntry*, std::pair<CacheIndices, MD5>> nodes;
	for (auto& node : m_songlist)
	{
		const std::pair<CacheIndices, MD5> base = { {} , node.key };
		for (auto& entry : *node)
			nodes.insert({ &entry, base });
	}

	BufferedBinaryWriter writer("songcache.bin");
	writer.write(s_CACHE_VERSION);
	m_categories.title.mapToCache(writer, nodes);
	m_categories.artist.mapToCache(writer, nodes);
	m_categories.album.mapToCache(writer, nodes);
	m_categories.genre.mapToCache(writer, nodes);
	m_categories.year.mapToCache(writer, nodes);
	m_categories.charter.mapToCache(writer, nodes);
	m_categories.playlist.mapToCache(writer, nodes);

	writer.write((uint32_t)nodes.size());
	for (const auto& node : nodes)
	{
		writer.startBuffer();
		node.first->serializeFileInfo(writer);
		writer.write(node.second.first);
		node.first->serializeSongInfo(writer);
		writer.write(node.second.second);
		writer.endBuffer();
	}
}

void SongLibrary::addEntry(MD5 hash, LibraryEntry&& entry)
{
	std::scoped_lock lock(m_entryMutex);
	m_songlist[hash].push_back(std::move(entry));
}

void SongLibrary::markScannedDirectory(const std::filesystem::path& directory)
{
	std::scoped_lock lock(m_directoryMutex);
	m_preScannedDirectories.insert(directory);
}

bool SongLibrary::findOrMarkDirectory(const std::filesystem::path& directory)
{
	std::scoped_lock lock(m_directoryMutex);
	if (m_preScannedDirectories.contains(directory))
		return true;

	m_preScannedDirectories.insert(directory);
	return false;
}
