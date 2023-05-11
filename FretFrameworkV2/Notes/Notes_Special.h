#pragma once
#include "Notes.h"

template <class NoteType, size_t numColors, class SpecialType>
class Note_withSpecial : public Note<NoteType, numColors>
{
protected:
	SpecialType m_special;

public:
	bool set(const size_t lane, uint64_t length)
	{
		if (lane == 0)
		{
			m_special.setLength(length);
			return true;
		}
		return Note<NoteType, numColors>::set(lane - 1, length);
	}

	SpecialType& getSpecial() noexcept
	{
		return m_special;
	}

	const SpecialType& getSpecial() const noexcept
	{
		return m_special;
	}

	bool validate() const noexcept
	{
		return m_special.isActive() || Note<NoteType, numColors>::validate();
	}

	std::vector<std::pair<size_t, uint64_t>> getActiveColors() const
	{
		std::vector<std::pair<size_t, uint64_t>> activeColors = Note<NoteType, numColors>::getActiveColors();
		for (auto& col : activeColors)
			col.first++;

		if (m_special.isActive())
			activeColors.insert(activeColors.begin(), { 0, m_special.getLength() });
		return activeColors;
	}

	uint64_t getLongestSustain() const noexcept
	{
		const uint64_t sustain = Note<NoteType, numColors>::getLongestSustain();
		return m_special.isActive() && m_special.getLength() > sustain ? m_special.getLength() : sustain;;
	}

	std::vector<std::tuple<unsigned char, unsigned char, uint32_t>> getMidiNotes() const noexcept
	{
		auto colors = Note<NoteType, numColors>::getMidiNotes();
		for (auto& col : colors)
			std::get<0>(col)++;
		if (m_special.isActive())
			colors.insert(colors.begin(), { 0, 100, (uint32_t)m_special.getLength() });
		return colors;
	}

	bool operator==(const Note_withSpecial& note) const
	{
		return m_special == note.m_special && Note<NoteType, numColors>::operator==(note);
	}

	bool operator!=(const Note_withSpecial& note) const
	{
		return !operator==(note);
	}

	void operator*=(float multiplier)
	{
		m_special *= multiplier;
		Note<NoteType, numColors>::operator*=(multiplier);
	}

	static bool TestIndex(size_t lane)
	{
		return lane <= numColors;
	}

	static constexpr size_t GetLaneCount()
	{
		return numColors + 1;
	}
};
