#pragma once
#include "Sustained.h"
enum class StringMode
{
	Normal,
	Ghost,
	Bend,
	Muted,
	Tapped,
	Harmonics,
	Pinch_Harmonics
};

template <size_t maxSize>
class Fret
{
	size_t m_value = SIZE_MAX;
public:
	bool set(size_t value)
	{
		if (value > maxSize)
			return false;

		m_value = value;
		return true;
	}

	void disable() noexcept
	{
		m_value = SIZE_MAX;
	}

	size_t get() const noexcept { return m_value; }
};

template <size_t numFrets>
class String : public Sustained
{
	Fret<numFrets> m_fret = SIZE_MAX;
	StringMode m_mode = StringMode::Normal;

public:
	bool set(size_t fret, size_t length, StringMode mode = StringMode::Normal)
	{
		if (!m_fret.set(fret))
			return false;

		setLength(length);
		setMode(mode);
		return true;
	}

	void setMode(StringMode mode) noexcept { m_mode = mode; }
	void disable() noexcept
	{
		m_fret.disable();
		m_mode = StringMode::Normal;
		Sustained::disable();
	}

	Fret& getFret() noexcept { return m_fret; }
	Fret getFret() const noexcept { return m_fret; }
	StringMode getMode() const noexcept { return m_mode; }
};

enum class StringEmphasis
{
	None,
	High,
	Middle,
	Low
};

template <size_t numFrets>
class GuitarNote_Pro
{
	String<numFrets> m_strings[6];
	bool m_isHOPO = false;
	bool m_forceNumbering = false;
	bool m_reverseSlide = false;
	StringEmphasis m_emphasis = StringEmphasis::None;
	bool m_palmMuted = false;
	bool m_vibrato = false;

public:
	String& get(size_t string) noexcept
	{
		return m_strings[string];
	}

	const String& get(size_t string) const noexcept
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
		case 'R':
			m_reverseSlide = true;
			return true;
		case 'E':
			switch (lane)
			{
			case 0:
				m_emphasis = StringEmphasis::None;
				break;
			case 1:
				m_emphasis = StringEmphasis::High;
				break;
			case 2:
				m_emphasis = StringEmphasis::Middle;
				break;
			case 3:
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

	void setHOPO(bool isHopo) noexcept { m_isHOPO = isHopo; }
	void setForcedNumbering(bool active) noexcept { m_forceNumbering = active; }
	void setSlideDirection(bool reverse) noexcept { m_reverseSlide = reverse; }
	void setStringEmphasis(StringEmphasis string) noexcept { m_emphasis = string; }
	void setPalmMuted(bool active) noexcept { m_palmMuted = active; }
	void setVibrato(bool active) noexcept { m_vibrato = active; }
	bool isHOPO() const noexcept { return m_isHOPO; }
	bool hasForcedNumbering() const noexcept { return m_forceNumbering; }
	bool hasReversedSlide() const noexcept { return m_reverseSlide; }
	StringEmphasis getStringEmphasis() const noexcept { return m_emphasis; }
	bool isPalmMuted() const noexcept { return m_palmMuted; }
	bool isVibrato() const noexcept { return m_vibrato; }

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
};

template <size_t numFrets>
class ArpeggioNote : public Sustained
{
	Fret<numFrets> m_strings[6];
public:
	Fret& get(size_t string) noexcept
	{
		return m_strings[string];
	}

	const Fret& get(size_t string) const noexcept
	{
		return m_strings[string];
	}
};

