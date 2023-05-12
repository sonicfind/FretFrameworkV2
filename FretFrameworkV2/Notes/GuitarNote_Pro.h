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

template <size_t numFrets>
class GuitarNote_Pro
{
	String<numFrets> m_strings[6];
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
};

