#pragma once
#include "Pitch.h"

using ProKeys_Pitch = Pitch<3, 5>;
class Keys_Pro
{
	ProKeys_Pitch m_pitches[4];
	size_t m_numActiveNotes = 0;
public:
	bool set(ProKeys_Pitch&& pitch)
	{
		if (m_numActiveNotes >= 4)
			return false;

		m_pitches[m_numActiveNotes++] = std::move(pitch);
		return true;
	}

	bool set(size_t index, ProKeys_Pitch&& pitch)
	{
		if (index < m_numActiveNotes)
			m_pitches[index] = std::move(pitch);
		else if (index == m_numActiveNotes && m_numActiveNotes != 4)
			m_pitches[m_numActiveNotes++] = std::move(pitch);
		else
			return false;
		return true;
	}

	void setLength(size_t index, uint64_t length)
	{
		if (index >= m_numActiveNotes)
			return;

		m_pitches[index].setLength(length);
	}

	bool setPitch(size_t index, char value)
	{
		if (index >= m_numActiveNotes)
			return false;

		return m_pitches[index].set(value);
	}

	bool setPitch(size_t index, NoteName note, int octave)
	{
		if (index >= m_numActiveNotes)
			return false;

		return m_pitches[index].set(note, octave);
	}

	size_t disable(size_t index)
	{
		if (index >= m_numActiveNotes)
			return m_numActiveNotes;

		while (index + 1 < m_numActiveNotes)
		{
			m_pitches[index] = std::move(m_pitches[index + 1]);
			++index;
		}
		m_pitches[index].disable();
		return --m_numActiveNotes;
	}

	size_t getNumActiveNotes() const noexcept { return m_numActiveNotes; }

	const ProKeys_Pitch& operator[](size_t index) const
	{
		if (index >= m_numActiveNotes)
			throw std::runtime_error("Index extends past number of active notes");
		return m_pitches[index];
	}
};

enum class ProKey_Ranges
{
	C1_E2,
	D1_F2,
	E1_G2,
	F1_A2,
	G1_B2,
	A1_C3,
};
