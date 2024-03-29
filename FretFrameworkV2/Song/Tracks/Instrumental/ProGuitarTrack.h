#pragma once
#include "InstrumentalTrack.h"
#include "ProGuitarDifficulty.h"
#include "Notes/NoteName.h"

enum class ChordPhrase
{
	Force_Numbering,
	Slash,
	Hide,
	Accidental_Switch
};

template <int numFrets>
class InstrumentalTrack<GuitarNote_Pro<numFrets>> : public Track
{
	class LeftHandPosition
	{
		int m_position;
	public:
		LeftHandPosition() = default;
		LeftHandPosition(int position)
		{
			if (!set(position))
				throw std::runtime_error("Invalid left hand position");
		}

		bool set(int position)
		{
			if (position == 0 || position > numFrets)
				return false;
			m_position = position;
			return true;
		}

		int getPosition() const noexcept { return m_position; }
	};

public:
	DifficultyTrack<GuitarNote_Pro<numFrets>> m_difficulties[4];
	SimpleFlatMap<NoteName> m_roots;
	SimpleFlatMap<LeftHandPosition> m_handPositions;
	SimpleFlatMap<std::vector<ChordPhrase>> m_chordPhrases;

	[[nodiscard]] virtual bool isOccupied() const override
	{
		for (const auto& diff : m_difficulties)
			if (diff.isOccupied())
				return true;
		return !m_roots.isEmpty() || !m_handPositions.isEmpty() || !m_chordPhrases.isEmpty() || Track::isOccupied();
	}

	virtual void clear() override
	{
		Track::clear();
		for (auto& diff : m_difficulties)
			diff.clear();
		m_roots.clear();
		m_handPositions.clear();
		m_chordPhrases.clear();
	}

	virtual void adjustTicks(float multiplier)
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);

		for (auto& root : m_roots)
			root.key = uint64_t(root.key * multiplier);

		for (auto& hand : m_handPositions)
			hand.key = uint64_t(hand.key * multiplier);

		for (auto& vec : m_chordPhrases)
			vec.key = uint64_t(vec.key * multiplier);
	}

	void shrink()
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

	DifficultyTrack<GuitarNote_Pro<numFrets>>& operator[](size_t i)
	{
		return m_difficulties[i];
	}

	const DifficultyTrack<GuitarNote_Pro<numFrets>>& operator[](size_t i) const noexcept
	{
		return m_difficulties[i];
	}
};
