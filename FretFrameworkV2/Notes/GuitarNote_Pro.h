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

enum class StringEmphasis
{
	None,
	High,
	Middle,
	Low
};

enum class SlideDirection
{
	None,
	Normal,
	Reversed
};

template <int numFrets>
class GuitarNote_Pro
{
	bool m_isHOPO = false;
	bool m_forceNumbering = false;
	SlideDirection m_slide = SlideDirection::None;
	StringEmphasis m_emphasis = StringEmphasis::None;
	bool m_palmMuted = false;
	bool m_vibrato = false;

public:
	String<numFrets> m_strings[6];

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
		case 'S':
			switch (lane)
			{
			case 0:
				m_slide = SlideDirection::Normal;
				break;
			case 1:
				m_slide = SlideDirection::Reversed;
				break;
			}
			return true;
		case 'E':
			switch (lane)
			{
			case 0:
				m_emphasis = StringEmphasis::High;
				break;
			case 1:
				m_emphasis = StringEmphasis::Middle;
				break;
			case 2:
				m_emphasis = StringEmphasis::Low;
				break;
			}
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
	GuitarNote_Pro& setSlideDirection(SlideDirection slide) noexcept { m_slide = slide; return *this; }
	GuitarNote_Pro& setStringEmphasis(StringEmphasis string) noexcept { m_emphasis = string; return *this; }
	GuitarNote_Pro& setPalmMuted(bool active) noexcept { m_palmMuted = active; return *this; }
	GuitarNote_Pro& setVibrato(bool active) noexcept { m_vibrato = active; return *this; }
	bool isHOPO() const noexcept { return m_isHOPO; }
	bool hasForcedNumbering() const noexcept { return m_forceNumbering; }
	SlideDirection getSlideDirection() const noexcept { return m_slide; }
	StringEmphasis getStringEmphasis() const noexcept { return m_emphasis; }
	bool isPalmMuted() const noexcept { return m_palmMuted; }
	bool isVibrato() const noexcept { return m_vibrato; }

	SlideDirection wheelSlideStatus() noexcept
	{
		if (m_slide == SlideDirection::None)
			m_slide = SlideDirection::Normal;
		else if (m_slide == SlideDirection::Normal)
			m_slide = SlideDirection::Reversed;
		else
			m_slide = SlideDirection::None;
		return m_slide;
	}

	StringEmphasis wheelEmphasis() noexcept
	{
		if (m_emphasis == StringEmphasis::None)
			m_emphasis = StringEmphasis::High;
		else if (m_emphasis == StringEmphasis::High)
			m_emphasis = StringEmphasis::Middle;
		else if (m_emphasis == StringEmphasis::Middle)
			m_emphasis = StringEmphasis::Low;
		else
			m_emphasis = StringEmphasis::None;
		return m_emphasis;
	}

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


template <int numFrets>
class LeftHandPosition
{
	int m_position;
public:
	bool set(int position)
	{
		if (position == 0 || position > numFrets - 3)
			return false;
		m_position = position;
		return true;
	}

	int getPosition() const noexcept { return m_position; }
};
