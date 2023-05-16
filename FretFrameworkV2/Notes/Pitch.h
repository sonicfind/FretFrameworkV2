#pragma once
#include <utility>
#include <stdexcept>
#include "Sustained.h"
#include "NoteName.h"

enum class PitchType
{
	In_Scale,
	Sharp,
	Flat
};

template <int OCTAVE_MIN = -1, int OCTAVE_MAX = 6>
class Pitch : public Sustained<true>
{
	static_assert(OCTAVE_MIN < OCTAVE_MAX && OCTAVE_MIN >= -1 && OCTAVE_MAX <= 9);

	NoteName m_note = NoteName::C;
	int m_octave = -1;
public:
	Pitch() = default;
	Pitch(const Pitch&) = default;
	Pitch(Pitch&&) = default;
	Pitch& operator=(const Pitch&) = default;
	Pitch& operator=(Pitch&&) = default;
	
	constexpr Pitch(NoteName note, int octave = OCTAVE_MIN)
	{
		if (!set(note, octave))
			throw std::runtime_error("Pitch received is out of range");
	}

	constexpr Pitch(char binaryValue, uint64_t length)
	{
		if (!set(binaryValue, length))
			throw std::runtime_error("Pitch received is out of range");
	}

	template <int other_min, int other_max>
	constexpr Pitch(const Pitch<other_min, other_max>& pitch) : Pitch(pitch.getNote(), pitch.getOctave()) {}

	Pitch& operator=(char binaryValue)
	{
		if (!set(binaryValue))
			throw std::runtime_error("Pitch received is out of range");
		return *this;
	}

	template <int other_min, int other_max>
	Pitch& operator=(const Pitch<other_min, other_max>& pitch)
	{
		if (!set(pitch))
			throw std::runtime_error("Pitch received is out of range");
		return *this;
	}

	[[nodiscard]] constexpr bool set(NoteName note, int octave) noexcept
	{ 
		const int origOctave = m_octave;
		if (!setOctave(octave) || !setNote(note))
		{
			m_octave = origOctave;
			return false;
		}
		return true;
	}

	template <int other_min, int other_max>
	[[nodiscard]] constexpr bool set(const Pitch<other_min, other_max>& pitch) noexcept
	{
		const int origOctave = m_octave;
		if (!set(pitch.getNote(), pitch.getOctave()))
		{
			m_octave = origOctave;
			return false;
		}
		return true;
	}

	constexpr bool set(char binaryValue) noexcept { return binaryValue >= 0 && set(static_cast<NoteName>(binaryValue % OCTAVE_LENGTH), (binaryValue / OCTAVE_LENGTH) - 1); }
	constexpr bool set(char binaryValue, uint64_t length) noexcept
	{
		if (!set(binaryValue))
			return false;
		setLength(length);
		return true;
	}

	[[nodiscard]] constexpr bool setOctave(int octave) noexcept
	{
		if (octave < OCTAVE_MIN || octave > OCTAVE_MAX)
			return false;

		m_octave = octave;
		return true;
	}

	[[nodiscard]] constexpr bool setNote(NoteName note) noexcept
	{
		if (m_octave == OCTAVE_MAX)
		{
			if constexpr (OCTAVE_MAX < 9)
			{
				if (note > NoteName::C)
					return false;
			}
			else if (note > NoteName::G)
				return false;
		}

		m_note = note;
		return true;
	}

	constexpr void reset() noexcept
	{
		m_note = NoteName::C;
		m_octave = -1;
	}

	[[nodiscard]] constexpr NoteName getNote() const noexcept { return m_note; }
	[[nodiscard]] constexpr int getOctave() const noexcept { return m_octave; }
	[[nodiscard]] constexpr char getBinaryValue() const noexcept { return (m_octave + 1) * OCTAVE_LENGTH + static_cast<char>(m_note); }
	[[nodiscard]] constexpr bool isActive() const noexcept { return Sustained::isActive() || m_octave > -1 || m_note > NoteName::C; }

	[[nodiscard]] constexpr static std::pair<int, int> getRange() { return { OCTAVE_MIN, OCTAVE_MAX }; }
};
