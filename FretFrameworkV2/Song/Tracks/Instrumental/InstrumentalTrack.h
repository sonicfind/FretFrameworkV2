#pragma once
#include "DifficultyTrack.h"
#include "Serialization/MidiFileReader.h"

template <class T>
class InstrumentalTrack : public Track
{
public:
	DifficultyTrack<T> m_difficulties[5];

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

	virtual void shrink() override
	{
		for (auto& diff : m_difficulties)
			diff.shrink();
	}

	virtual void clear() override
	{
		Track::clear();
		for (auto& diff : m_difficulties)
			diff.clear();
	}

	virtual void adjustTicks(float multiplier)
	{
		Track::adjustTicks(multiplier);
		for (auto& diff : m_difficulties)
			if (diff.isOccupied())
				diff.adjustTicks(multiplier);
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
