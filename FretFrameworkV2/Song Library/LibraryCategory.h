#pragma once
#include "Entry/LibraryEntry.h"
#include "SimpleFlatMap/SimpleFlatMap.h"

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

	void clear()
	{
		m_elements.clear();
	}
};
