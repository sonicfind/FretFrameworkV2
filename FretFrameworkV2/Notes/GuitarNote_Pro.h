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

