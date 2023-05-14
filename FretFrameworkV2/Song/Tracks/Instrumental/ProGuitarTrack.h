#pragma once
#include "InstrumentalTrack.h"
#include "ProGuitarDifficulty.h"
#include "Notes/NoteName.h"

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

	InstrumentalTrack() = default;
	InstrumentalTrack(InstrumentalTrack&&) = default;
	InstrumentalTrack& operator=(InstrumentalTrack&&) = default;

	InstrumentalTrack(const InstrumentalTrack&) = delete;
	InstrumentalTrack& operator=(const InstrumentalTrack&) = delete;

	[[nodiscard]] virtual bool isOccupied() const override
	{
		for (const auto& diff : m_difficulties)
			if (diff.isOccupied())
				return true;
		return !m_roots.isEmpty() || Track::isOccupied();
	}

	virtual void clear() override
	{
		Track::clear();
		for (auto& diff : m_difficulties)
			diff.clear();
		m_roots.clear();
	}

	virtual void adjustTicks(float multiplier)
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);

		for (auto& root : m_roots)
			root.key = uint64_t(root.key * multiplier);
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
