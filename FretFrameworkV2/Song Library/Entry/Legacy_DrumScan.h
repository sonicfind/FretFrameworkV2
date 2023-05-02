#pragma once
#include "InstrumentalScan.h"
#include "Notes/DrumNote_Legacy.h"

class Legacy_DrumScan : public InstrumentalScan<DrumNote_Legacy>
{
private:
	using InstrumentalScan<DrumNote_Legacy>::scan;

public:
	Legacy_DrumScan(DrumType_Enum type);
	Legacy_DrumScan(MidiFileReader& reader);

	[[nodiscard]] bool extraTest_V1(size_t note) override;
	[[nodiscard]] DrumType_Enum getDrumType() const noexcept { return m_type; }

	virtual void applyExtraValue(Midi_Scanner& scanner) override;

	template <size_t numPads, bool PRO_DRUMS>
	void transfer(InstrumentalScan_Extended<DrumNote<numPads, PRO_DRUMS>>& track) const noexcept
	{
		track.m_subTracks |= m_subTracks;
		if (m_intensity != 0xFF)
			track.m_intensity = m_intensity;
	}

private:
	DrumType_Enum m_type = DrumType_Enum::LEGACY;
};

template<>
struct InstrumentalScan<DrumNote_Legacy>::Midi_Scanner_Extensions
{
	DrumType_Enum type = DrumType_Enum::LEGACY;
};

template<>
constexpr bool InstrumentalScan<DrumNote_Legacy>::Midi_Scanner::s_doExtra = true;

template <>
constexpr std::pair<unsigned char, unsigned char> InstrumentalScan<DrumNote_Legacy>::s_noteRange{ 60, 102 };

template <>
template <bool NoteOn>
void InstrumentalScan<DrumNote_Legacy>::parseLaneColor(Midi_Scanner& scanner, MidiNote note)
{
	if (note.value == 95)
	{
		if constexpr (!NoteOn)
		{
			if (scanner.difficulties[3].notes[0])
			{
				m_subTracks |= 8;
				scanner.difficulties[3].active = true;
			}
		}
		scanner.difficulties[3].notes[0] = true;
	}
	else
	{
		const int noteValue = note.value - s_noteRange.first;
		const int diff = s_diffValues[noteValue];
		const int lane = scanner.laneValues[noteValue];
		if (!scanner.difficulties[diff].active)
		{
			if (lane < 6)
			{
				if constexpr (!NoteOn)
				{
					if (scanner.difficulties[diff].notes[lane])
					{
						m_subTracks |= 1 << diff;
						scanner.difficulties[diff].active = true;
					}
				}
				scanner.difficulties[diff].notes[lane] = true;
			}
		}

		if (lane == 5 && scanner.ext.type == DrumType_Enum::LEGACY)
		{
			scanner.ext.type = DrumType_Enum::FIVELANE;
			applyExtraValue(scanner);
		}
	}
}

template <>
template <bool NoteOn>
void InstrumentalScan<DrumNote_Legacy>::toggleExtraValues(Midi_Scanner& scanner, MidiNote note)
{
	if (110 <= note.value && note.value <= 112)
	{
		scanner.ext.type = DrumType_Enum::FOURLANE_PRO;
		applyExtraValue(scanner);
	}
}
