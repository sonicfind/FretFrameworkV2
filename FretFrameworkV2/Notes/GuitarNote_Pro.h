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

enum class ProSlide
{
	None,
	Normal,
	Reversed
};

enum class EmphasisType
{
	None,
	High,
	Middle,
	Low
};

template <int numStrings, int numFrets>
class GuitarNote_Pro
{
public:
	String<numFrets> m_strings[numStrings];

private:
	bool m_isHOPO = false;
	bool m_forceNumbering = false;
	bool m_palmMuted = false;
	bool m_vibrato = false;
	ProSlide m_slide = ProSlide::None;
	EmphasisType m_emphasis = EmphasisType::None;

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
		case 'S':
			if (m_slide != ProSlide::None)
				break;

			switch (lane)
			{
			case 0: m_slide = ProSlide::Normal; break;
			case 1: m_slide = ProSlide::Reversed; break;
			}
			return true;
		case 'E':
			if (m_emphasis != EmphasisType::None)
				break;

			switch (lane)
			{
			case 0: m_emphasis = EmphasisType::High; break;
			case 1: m_emphasis = EmphasisType::Middle; break;
			case 2: m_emphasis = EmphasisType::Low; break;
			}
			return true;
		default:
			return false;
		}
	}

	GuitarNote_Pro& setHOPO(bool isHopo) noexcept { m_isHOPO = isHopo; return *this; }
	GuitarNote_Pro& setForcedNumbering(bool active) noexcept { m_forceNumbering = active; return *this; }
	GuitarNote_Pro& setPalmMuted(bool active) noexcept { m_palmMuted = active; return *this; }
	GuitarNote_Pro& setVibrato(bool active) noexcept { m_vibrato = active; return *this; }
	GuitarNote_Pro& setSlide(ProSlide slide) noexcept { m_slide = slide; return *this; }
	GuitarNote_Pro& setEmphasis(EmphasisType emphasis) noexcept { m_emphasis = emphasis; return *this; }
	bool isHOPO() const noexcept { return m_isHOPO; }
	bool hasForcedNumbering() const noexcept { return m_forceNumbering; }
	bool isPalmMuted() const noexcept { return m_palmMuted; }
	bool isVibrato() const noexcept { return m_vibrato; }
	ProSlide getSliode() const noexcept { return m_slide; }
	EmphasisType getEmphasis() const noexcept { return m_emphasis; }

	ProSlide wheelSlide() noexcept
	{
		if (m_slide == ProSlide::None)
			m_slide = ProSlide::Normal;
		else if (m_slide == ProSlide::Normal)
			m_slide = ProSlide::Reversed;
		else
			m_slide = ProSlide::None;
		return m_slide;
	}

	EmphasisType wheelEmphasis() noexcept
	{
		if (m_emphasis == EmphasisType::None)
			m_emphasis = EmphasisType::High;
		else if (m_emphasis == EmphasisType::High)
			m_emphasis = EmphasisType::Middle;
		else if (m_emphasis == EmphasisType::Middle)
			m_emphasis = EmphasisType::Low;
		else
			m_emphasis = EmphasisType::None;
		return m_emphasis;
	}

	void operator*=(double multiplier)
	{
		for (auto& string : m_strings)
			string *= multiplier;
	}

	static constexpr size_t GetLaneCount() { return numStrings; }
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
