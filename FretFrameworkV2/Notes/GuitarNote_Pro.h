#pragma once
#include "Sustained.h"
enum class StringMode
{
	Normal,
	Bend,
	Muted,
	Tapped,
	Harmonics,
	Pinch_Harmonics
};

template <int maxSize>
class Fret
{
	int m_value = -1;
public:
	bool set(int value)
	{
		if (value > maxSize)
			return false;

		m_value = value;
		return true;
	}

	void disable() noexcept
	{
		m_value = -1;
	}

	int get() const noexcept { return m_value; }
	bool isActive() const noexcept { return m_value != -1; }
};

template <int numFrets>
class String : public Sustained<true>
{
public:
	Fret<numFrets> m_fret;

private:
	StringMode m_mode = StringMode::Normal;

public:
	void setMode(StringMode mode) noexcept { m_mode = mode; }
	StringMode getMode() const noexcept { return m_mode; }

	void disable() noexcept
	{
		Sustained::disable();
		m_fret.disable();
		m_mode = StringMode::Normal;
	}

	bool isActive() const noexcept { return Sustained::isActive() && m_fret.isActive(); }
};

template <int numFrets>
class GuitarNote_Pro
{
public:
	String<numFrets> m_strings[6];

private:
	bool m_isHOPO = false;
	bool m_forceNumbering = false;
	bool m_palmMuted = false;
	bool m_vibrato = false;

public:
	String<numFrets>& operator[](size_t string) noexcept
	{
		return m_strings[string];
	}

	const String<numFrets>& operator[](size_t string) const noexcept
	{
		return m_strings[string];
	}

	bool modify(char modifier, size_t lane = 0) noexcept
	{
		switch (modifier)
		{
		case 'H':
			m_isHOPO = true;
			return true;
		case 'F':
			m_forceNumbering = true;
			return true;
		case 'P':
			m_palmMuted = true;
			return true;
		case 'V':
			m_vibrato = true;
			return true;
		default:
			return false;
		}
	}

	GuitarNote_Pro& setHOPO(bool isHopo) noexcept { m_isHOPO = isHopo; return *this; }
	GuitarNote_Pro& setForcedNumbering(bool active) noexcept { m_forceNumbering = active; return *this; }
	GuitarNote_Pro& setPalmMuted(bool active) noexcept { m_palmMuted = active; return *this; }
	GuitarNote_Pro& setVibrato(bool active) noexcept { m_vibrato = active; return *this; }
	bool isHOPO() const noexcept { return m_isHOPO; }
	bool hasForcedNumbering() const noexcept { return m_forceNumbering; }
	bool isPalmMuted() const noexcept { return m_palmMuted; }
	bool isVibrato() const noexcept { return m_vibrato; }

	void operator*=(double multiplier)
	{
		for (auto& string : m_strings)
			string *= multiplier;
	}
};

template <int numFrets>
class ArpeggioNote : public Sustained<false>
{
public:
	Fret<numFrets> m_strings[6];

public:
	Fret<numFrets>& operator[](size_t string) noexcept
	{
		return m_strings[string];
	}

	const Fret<numFrets>& operator[](size_t string) const noexcept
	{
		return m_strings[string];
	}
};

enum class EmphasisType
{
	High,
	Middle,
	Low
};

class StringEmphasis : public Sustained<false>
{
	EmphasisType m_string = EmphasisType::High;
public:
	StringEmphasis() = default;
	StringEmphasis(EmphasisType string) : m_string(string) {}
	StringEmphasis& setEmphasis(EmphasisType string) { m_string = string; return *this; }
	EmphasisType getEmphasis() const noexcept { return m_string; }

	StringEmphasis wheelEmphasis() noexcept
	{
		if (m_string == EmphasisType::High)
			m_string = EmphasisType::Middle;
		else if (m_string == EmphasisType::Middle)
			m_string = EmphasisType::Low;
		else
			m_string = EmphasisType::High;
		return m_string;
	}
};

class ProSlide : public Sustained<false>
{
	bool m_isReversed = false;
public:
	ProSlide() = default;
	ProSlide(bool isReverse) : m_isReversed(isReverse) {}
	ProSlide& setDirection(bool isReverse) { m_isReversed = isReverse; return *this; }
	bool isReversed() const noexcept { return m_isReversed; }
};
