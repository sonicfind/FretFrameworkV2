#pragma once
#include "Notes_Special.h"

enum class DrumType_Enum
{
	LEGACY,
	FOURLANE_PRO,
	FIVELANE,
};

enum class DrumDynamics
{
	None,
	Accent,
	Ghost
};

template <bool PRO_DRUMS>
class DrumPad : public NoteColor
{
protected:
	DrumDynamics dynamics = DrumDynamics::None;

public:
	void disable()
	{
		NoteColor::disable();
		dynamics = DrumDynamics::None;
	}

	bool modify(char modifier)
	{
		switch (modifier)
		{
		case 'A':
			dynamics = DrumDynamics::Accent;
			return true;
		case 'G':
			dynamics = DrumDynamics::Ghost;
			return true;
		default:
			return false;
		}
	}

	DrumDynamics getDynamics() const noexcept { return dynamics; }
	void wheelDynamics()
	{
		if (dynamics == DrumDynamics::None)
			dynamics = DrumDynamics::Accent;
		else if (dynamics == DrumDynamics::Accent)
			dynamics = DrumDynamics::Ghost;
		else
			dynamics = DrumDynamics::None;
	}
	void setDynamics(DrumDynamics dyn) { dynamics = dyn; }

	std::vector<char> getActiveModifiers() const noexcept
	{
		switch (dynamics)
		{
		case DrumDynamics::Accent:
			return { 'A' };
		case DrumDynamics::Ghost:
			return { 'G' };
		};
		return {};
	}
};

template<>
class DrumPad<true> : public DrumPad<false>
{
	bool m_isCymbal = false;
public:
	void disable()
	{
		DrumPad<false>::disable();
		m_isCymbal = false;
	}

	bool modify(char modifier)
	{
		if (modifier == 'C')
		{
			m_isCymbal = true;
			return true;
		}
		return DrumPad<false>::modify(modifier);
	}

	bool isCymbal() const noexcept { return m_isCymbal; }
	void toggleCymbal() { m_isCymbal = !m_isCymbal; }
	void setCymbal(bool enable) { m_isCymbal = enable; }

	std::vector<char> getActiveModifiers() const noexcept
	{
		auto modifiers = DrumPad<false>::getActiveModifiers();
		if (m_isCymbal)
			modifiers.push_back('C');
		return modifiers;
	}
};


template <size_t numPads, bool PRO_DRUMS>
class DrumNote : public Note_withSpecial<DrumPad<PRO_DRUMS>, numPads, NoteColor>
{
protected:
	using Note_withSpecial<DrumPad<PRO_DRUMS>, numPads, NoteColor>::m_colors;
	using Note_withSpecial<DrumPad<PRO_DRUMS>, numPads, NoteColor>::m_special;

	bool m_isDoubleBass = false;
	bool m_isFlammed = false;

public:
	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (lane == 0)
			m_special.set(sustain);
		else if (lane <= numPads && lane < 32)
			m_colors[lane - 1].set(sustain);
		else if (lane == 32)
			toggleDoubleBass();
		else if (34 <= lane && lane < 34 + numPads)
			m_colors[lane - 34].setDynamics(DrumDynamics::Accent);
		else if (40 <= lane && lane < 40 + numPads)
			m_colors[lane - 40].setDynamics(DrumDynamics::Accent);
		else if constexpr (PRO_DRUMS)
		{
			if (lane < 66 || lane > 68)
				return false;

			m_colors[lane - 65].toggleCymbal();
		}
		else
			return false;
		return true;
	}

	bool modify(char modifier, size_t lane = 0)
	{
		switch (modifier)
		{
		case '+':
			toggleDoubleBass();
			return true;
		case 'F':
			toggleFlam();
			return true;
		default:
			if (0 < lane && lane <= numPads)
				return m_colors[lane - 1].modify(modifier);
			return false;
		}
	}

	std::vector<std::pair<char, size_t>> getActiveModifiers() const
	{
		std::vector<std::pair<char, size_t>> modifiers;
		if (m_isDoubleBass)
			modifiers.push_back({ '+', SIZE_MAX });

		if (m_isFlammed)
			modifiers.push_back({ 'F', SIZE_MAX });

		for (size_t i = 0; i < numPads; ++i)
			for (const char mod : m_colors[i].getActiveModifiers())
				modifiers.push_back({ mod, i + 1 });
		return modifiers;
	}

	std::vector<char> getActiveModifiers(size_t index) const
	{
		std::vector<char> modifiers;
		if (m_isDoubleBass)
			modifiers.push_back('+');

		if (m_isFlammed)
			modifiers.push_back('F');

		for (const char mod : m_colors[index].getActiveModifiers())
			modifiers.push_back(mod);
		return modifiers;
	}

	std::vector<std::tuple<char, char, uint32_t>> getMidiNotes() const noexcept
	{
		auto colors = Note<DrumPad<PRO_DRUMS>, numPads>::getMidiNotes();
		if (m_special.isActive() && !m_isDoubleBass)
			colors.insert(colors.begin(), { 0, 100, m_special.getSustain() });

		for (std::tuple<char, char, uint32_t>& col : colors)
		{
			size_t index = std::get<0>(col);
			if (index != 0)
			{
				switch (this->get(index).getDynamics())
				{
				case DrumDynamics::Accent:
					std::get<1>(col) = 101;
					break;
				case DrumDynamics::Ghost:
					std::get<1>(col) = 1;
					break;
				}
			}
		}
		return colors;
	}

	void toggleDoubleBass()
	{
		m_isDoubleBass = !m_isDoubleBass;
	}

	void setDoubleBass(bool enable)
	{
		m_isDoubleBass = enable;
	}

	void toggleFlam()
	{
		m_isFlammed = !m_isFlammed;
	}

	void setFlam(bool enable)
	{
		m_isFlammed = enable;
	}

	[[nodiscard]] bool isDoubleBass() const noexcept { return m_isDoubleBass; }
	[[nodiscard]] bool isFlammed() const noexcept { return m_isFlammed; }

	static bool TestIndex_V1(const size_t lane)
	{
		return lane <= numPads;
	}
};
