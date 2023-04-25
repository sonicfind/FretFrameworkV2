#pragma once
#include "Notes_Special.h"

enum class DrumDynamics
{
	None,
	Accent,
	Ghost
};

class DrumPad : public NoteColor
{
	DrumDynamics dynamics = DrumDynamics::None;
	bool m_isCymbal = false;

public:
	template <bool PRO_DRUMS>
	bool modify(char modifier)
	{
		switch (modifier)
		{
		case 'C':
			if constexpr (!PRO_DRUMS)
				return false;

			toggleCymbal();
			return true;
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

	bool isCymbal() const noexcept { return m_isCymbal; }
	void toggleCymbal() { m_isCymbal = !m_isCymbal; }
	void setCymbal(bool enable) { m_isCymbal = enable; }

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
};


template <size_t numPads, bool PRO_DRUMS>
class DrumNote : public Note_withSpecial<DrumPad, numPads, NoteColor>
{
	bool m_isDoubleBass = false;
	bool m_isFlammed = false;

public:
	bool set_V1(const size_t lane, uint32_t sustain)
	{
		if (sustain < 20)
			sustain = 1;

		if (lane == 0)
			this->m_special.set(sustain);
		else if (lane <= numPads)
			this->m_colors[lane - 1].set(sustain);
		else if (lane == 32)
			toggleDoubleBass();
		else if constexpr (PRO_DRUMS)
		{
			if (lane < 66 || lane > 68)
				return false;

			this->m_colors[lane - 65].toggleCymbal();
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
				return this->m_colors[lane - 1].modify<PRO_DRUMS>(modifier);
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
		{
			if constexpr (PRO_DRUMS)
				if (this->m_colors[i].isCymbal())
					modifiers.push_back({ 'C', i + 1 });

			switch (this->m_colors[i].getDynamics())
			{
			case DrumDynamics::Accent:
				modifiers.push_back({ 'A', i + 1 });
				break;
			case DrumDynamics::Ghost:
				modifiers.push_back({ 'G', i + 1 });
				break;
			};
		}
		return modifiers;
	}

	std::vector<char> getActiveModifiers(size_t index) const
	{
		std::vector<char> modifiers;
		if (m_isDoubleBass)
			modifiers.push_back('+');

		if (m_isFlammed)
			modifiers.push_back('F');

		if constexpr (PRO_DRUMS)
			if (this->m_colors[index].isCymbal())
				modifiers.push_back('C');

		switch (this->m_colors[index].getDynamics())
		{
		case DrumDynamics::Accent:
			modifiers.push_back('A');
			break;
		case DrumDynamics::Ghost:
			modifiers.push_back('G');
			break;
		};
		return modifiers;
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

public:
	[[nodiscard]] bool isAccented(size_t lane) const noexcept { return lane > 0 && this->m_colors[lane - 1].getDyanmics() == DrumDynamics::Accent; }
	[[nodiscard]] bool isGhosted(size_t lane) const noexcept { return lane > 0 && this->m_colors[lane - 1].getDyanmics() == DrumDynamics::Ghost; }
	void setDynamics(size_t lane, DrumDynamics dyn) { if (lane > 0) this->m_colors[lane - 1].setDynamics(dyn); }

	template <typename = std::enable_if<PRO_DRUMS>>
	[[nodiscard]] bool isCymbal(size_t lane) const noexcept { return lane > 1 && this->m_colors[lane - 1].isCymbal(); }

	template <typename = std::enable_if<PRO_DRUMS>>
	void toggleCymbal(size_t lane) { if (lane > 1) this->m_colors[lane - 1].toggleCymbal(); }

	template <typename = std::enable_if<PRO_DRUMS>>
	void setCymbal(size_t lane, bool enable) { if (lane > 1) this->m_colors[lane - 1].setCymbal(enable); }

public:
	static bool TestIndex_V1(const size_t lane)
	{
		return lane <= numPads;
	}
};
