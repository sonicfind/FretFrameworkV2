#pragma once
#include "DifficultyTrack.h"

template <class T>
class InstrumentalTrack : public Track
{
public:
	DifficultyTrack<T> m_difficulties[4];

public:
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
		return Track::isOccupied();
	}

	virtual void clear() override
	{
		Track::clear();
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	virtual void adjustTicks(double multiplier) override
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);
	}

	void shrink()
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

	DifficultyTrack<T>& operator[](size_t i)
	{
		return m_difficulties[i];
	}

	const DifficultyTrack<T>& operator[](size_t i) const noexcept
	{
		return m_difficulties[i];
	}
};
