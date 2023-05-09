#pragma once
#include "Notes_Special.h"
#include "DrumType.h"

enum class DrumDynamics
{
	None,
	Accent,
	Ghost
};

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

class DrumPad_Pro : public DrumPad
{
	bool m_isCymbal = false;
public:
	void disable()
	{
		DrumPad::disable();
		m_isCymbal = false;
	}

	bool modify(char modifier)
	{
		if (modifier == 'C')
		{
			m_isCymbal = true;
			return true;
		}
		return DrumPad::modify(modifier);
	}

	bool isCymbal() const noexcept { return m_isCymbal; }
	void toggleCymbal() { m_isCymbal = !m_isCymbal; }
	void setCymbal(bool enable) { m_isCymbal = enable; }

	std::vector<char> getActiveModifiers() const noexcept
	{
		auto modifiers = DrumPad::getActiveModifiers();
		if (m_isCymbal)
			modifiers.push_back('C');
		return modifiers;
	}
};


template <class DrumType, size_t numPads>
class DrumNote : public Note_withSpecial<DrumType, numPads, NoteColor>
{
protected:
	using Note_withSpecial<DrumType, numPads, NoteColor>::m_colors;
	using Note_withSpecial<DrumType, numPads, NoteColor>::m_special;
	NoteColor m_doubleBass;

	bool m_isFlammed = false;

public:
	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (lane == 0)
		{
			m_special.set(sustain);
			m_doubleBass.disable();
		}
		else if (lane <= numPads && lane < 32)
			m_colors[lane - 1].set(sustain);
		else if (lane == 32)
		{
			if (m_special.isActive())
				m_doubleBass = std::move(m_special);
		}
		else if (34 <= lane && lane < 34 + numPads)
			m_colors[lane - 34].setDynamics(DrumDynamics::Accent);
		else if (40 <= lane && lane < 40 + numPads)
			m_colors[lane - 40].setDynamics(DrumDynamics::Ghost);
		else if constexpr (std::is_same<DrumType, DrumPad_Pro>::value)
		{
			if (lane < 66 || lane > 68)
				return false;

			m_colors[lane - 65].setCymbal(true);
		}
		else
			return false;
		return true;
	}

	bool set(size_t lane, uint32_t sustain)
	{
		if (lane == 1)
		{
			m_doubleBass.set(sustain);
			this->m_special.disable();
			return true;
		}
		else
		{
			if (lane == 0)
				m_doubleBass.disable();
			else if (lane > 1)
				lane--;

			return Note_withSpecial<DrumType, numPads, NoteColor>::set(lane, sustain);
		}
	}

	NoteColor& getDoubleBass() noexcept
	{
		return m_doubleBass;
	}

	const NoteColor& getDoubleBass() const noexcept
	{
		return m_doubleBass;
	}

	bool modify(char modifier, size_t lane = 0)
	{
		switch (modifier)
		{
		case 'F':
			m_isFlammed = true;
			return true;
		default:
			if (2 <= lane && lane < numPads + 1)
				return m_colors[lane - 2].modify(modifier);
			return false;
		}
	}

	bool validate() const noexcept
	{
		return m_doubleBass.isActive() || Note_withSpecial<DrumType, numPads, NoteColor>::validate();
	}

	std::vector<std::pair<size_t, uint32_t>> getActiveColors() const
	{
		std::vector<std::pair<size_t, uint32_t>> activeColors = Note_withSpecial<DrumType, numPads, NoteColor>::getActiveColors();
		for (auto& col : activeColors)
			if (col.first > 0)
				col.first++;

		if (m_doubleBass.isActive())
			activeColors.insert(activeColors.begin(), { 1, m_doubleBass.getSustain() });
		return activeColors;
	}

	std::vector<std::pair<char, size_t>> getActiveModifiers() const
	{
		std::vector<std::pair<char, size_t>> modifiers;
		if (m_isFlammed)
			modifiers.push_back({ 'F', SIZE_MAX });

		for (size_t i = 0; i < numPads; ++i)
			for (const char mod : m_colors[i].getActiveModifiers())
				modifiers.push_back({ mod, i + 2 });
		return modifiers;
	}

	std::vector<char> getActiveModifiers(size_t index) const
	{
		std::vector<char> modifiers;
		if (m_isFlammed)
			modifiers.push_back('F');

		for (const char mod : m_colors[index].getActiveModifiers())
			modifiers.push_back(mod);
		return modifiers;
	}

	std::vector<std::tuple<char, char, uint32_t>> getMidiNotes() const noexcept
	{
		auto colors = Note_withSpecial<DrumType, numPads, NoteColor>::getMidiNotes();
		for (std::tuple<char, char, uint32_t>& col : colors)
		{
			size_t index = std::get<0>(col);
			if (index > 0)
			{
				switch (this->get(index - 1).getDynamics())
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

		if (m_doubleBass.isActive())
			colors.push_back({ 95, 100, m_doubleBass.getSustain() });
		return colors;
	}

	void toggleDoubleBass()
	{
		if (m_special.isActive())
			m_doubleBass = std::move(m_special);
		else if (m_doubleBass.isActive())
			m_special = std::move(m_doubleBass);
	}

	void toggleFlam()
	{
		m_isFlammed = !m_isFlammed;
	}

	void setFlam(bool enable)
	{
		m_isFlammed = enable;
	}

	[[nodiscard]] bool isFlammed() const noexcept { return m_isFlammed; }

	static bool TestIndex_V1(const size_t lane)
	{
		return lane <= numPads;
	}

	static bool TestIndex(const size_t lane)
	{
		return lane <= numPads + 1;
	}
};
