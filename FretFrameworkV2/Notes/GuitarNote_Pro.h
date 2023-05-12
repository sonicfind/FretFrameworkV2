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

template <size_t numFrets>
class String : public Sustained
{
	size_t m_fret = SIZE_MAX;
	StringMode m_mode = StringMode::Normal;

public:
	bool setFret(size_t fret)
	{
		if (fret > numFrets)
			return false;
		m_fret = fret;
		return true;
	}

	void setMode(StringMode mode) { m_mode = mode; }
	void disable()
	{
		m_fret = SIZE_MAX;
		m_mode = StringMode::Normal;
		Sustained::disable();
	}

	size_t getFret() const noexcept { return m_fret; }
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

public:
	bool set(size_t string, size_t fret, size_t length, StringMode mode)
	{
		if (string >= 6 && !m_strings[string].setFret(fret))
			return false;

		m_strings[string].setLength(length);
		m_strings[string].setMode(mode);
		return true;
	}

	String& get(size_t string) noexcept
	{
		return m_strings[string];
	}

	const String& get(size_t string) const noexcept
	{
		return m_strings[string];
	}

	void setHOPO(bool isHopo) { m_isHOPO = isHopo; }
	void setForcedNumbering(bool active) { m_forceNumbering = active; }
	void setSlideDirection(bool reverse) { m_reverseSlide = reverse; }
	void setStringEmphasis(StringEmphasis string) { m_emphasis = string; }
	bool isHOPO() const noexcept { return m_isHOPO; }
	bool hasForcedNumbering() const noexcept { return m_forceNumbering; }
	bool hasReversedSlide() const noexcept { return m_reverseSlide; }
	StringEmphasis getStringEmphasis() const noexcept { return m_emphasis; }
	StringEmphasis wheelEmphasis()
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

