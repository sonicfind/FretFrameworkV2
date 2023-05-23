#pragma once
#include "Entry/LibraryEntry.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Serialization/BufferedBinaryWriter.h"
#include <unordered_map>
#include <mutex>

struct CacheIndices
{
	uint32_t nameIndex = UINT32_MAX;
	uint32_t artistIndex = UINT32_MAX;
	uint32_t albumIndex = UINT32_MAX;
	uint32_t genreIndex = UINT32_MAX;
	uint32_t yearIndex = UINT32_MAX;
	uint32_t charterIndex = UINT32_MAX;
	uint32_t playlistIndex = UINT32_MAX;
};

template <SongAttribute Attribute>
class CategoryNode
{
	std::vector<PointerWrapper<LibraryEntry>> m_songs;

public:
	void add(PointerWrapper<LibraryEntry> song)
	{
		auto iter = std::lower_bound(m_songs.begin(), m_songs.end(), song,
			[](const PointerWrapper<LibraryEntry>& element, const PointerWrapper<LibraryEntry>& sng)
			{
				return element->isBelow<Attribute>(*sng);
			});

		m_songs.insert(iter, song);
	}

	auto begin() const noexcept { return m_songs.begin(); }
	auto end() const noexcept { return m_songs.end(); }

	void clear()
	{
		m_songs.clear();
	}
};

template <SongAttribute Attribute, class Element = CategoryNode<Attribute>>
class LibraryCategory
{
	SimpleFlatMap<Element, PointerWrapper<const UnicodeString>> m_elements;

public:
	void add(PointerWrapper<LibraryEntry> song)
	{
		m_elements[song->getAttribute<Attribute>()].add(song);
	}

	void mapToCache(BufferedBinaryWriter& writer, std::unordered_map<const LibraryEntry*, std::pair<CacheIndices, MD5>>& nodes) const
	{
		writer.append((uint32_t)m_elements.size());
		for (uint32_t i = 0; i < m_elements.size(); ++i)
		{
			const auto& element = m_elements.at_index(i);
			writer.appendString(element.key->toString());
			for (const auto& song : *element)
			{
				if      constexpr (Attribute == SongAttribute::ARTIST)   nodes.at(song.raw()).first.artistIndex = i;
				else if constexpr (Attribute == SongAttribute::ALBUM)    nodes.at(song.raw()).first.albumIndex = i;
				else if constexpr (Attribute == SongAttribute::GENRE)    nodes.at(song.raw()).first.genreIndex = i;
				else if constexpr (Attribute == SongAttribute::YEAR)     nodes.at(song.raw()).first.yearIndex = i;
				else if constexpr (Attribute == SongAttribute::CHARTER)  nodes.at(song.raw()).first.charterIndex = i;
				else if constexpr (Attribute == SongAttribute::PLAYLIST) nodes.at(song.raw()).first.playlistIndex = i;
			}
		}
		writer.writeBuffer();
	}

	auto begin() const noexcept { return m_elements.begin(); }
	auto end() const noexcept { return m_elements.end(); }

	void clear()
	{
		m_elements.clear();
	}
};

template <>
class LibraryCategory<SongAttribute::TITLE>
{
	SimpleFlatMap<CategoryNode<SongAttribute::TITLE>, char32_t> m_elements;

public:
	void add(PointerWrapper<LibraryEntry> song)
	{
		m_elements[song->getAttribute<SongAttribute::TITLE>().getLowerCase().front()].add(song);
	}

	void mapToCache(BufferedBinaryWriter& writer, std::unordered_map<const LibraryEntry*, std::pair<CacheIndices, MD5>>& nodes) const
	{
		std::vector<const UnicodeString*> strings;
		for (const auto& element : m_elements)
		{
			for (const auto& song : *element)
			{
				if (strings.empty() || strings.back()->get() != song->getName().get())
					strings.push_back(&song->getName());

				nodes.at(song.raw()).first.nameIndex = (uint32_t)strings.size() - 1;
			}
		}

		writer.append((uint32_t)strings.size());
		for (const auto& string : strings)
			writer.appendString(string->toString());
		writer.writeBuffer();
	}

	auto begin() const noexcept { return m_elements.begin(); }
	auto end() const noexcept { return m_elements.end(); }

	void clear()
	{
		m_elements.clear();
	}
};
