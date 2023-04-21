#pragma once
#include "Notes.h"

template <class NoteType, size_t numColors, class SpecialType>
class Note_withSpecial : public Note<NoteType, numColors>
{
protected:
	SpecialType m_special;

public:
	bool set(const size_t lane, uint32_t sustain)
	{
		if (lane == 0)
		{
			if (sustain < 20)
				sustain = 1;

			m_special.set(sustain);
			return true;
		}
		return Note<NoteType, numColors>::set(lane, sustain);
	}

	std::vector<std::pair<size_t, uint32_t>> getActiveColors() const
	{
		std::vector<std::pair<size_t, uint32_t>> activeColors = Note<NoteType, numColors>::getActiveColors();
		if (m_special.isActive())
			activeColors.insert(activeColors.begin(), { 0, m_special.getSustain() });
		return activeColors;
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

	static bool testIndex(size_t lane)
	{
		return lane <= numColors;
	}

	static constexpr size_t GetLaneCount()
	{
		return numColors + 1;
	}
};
