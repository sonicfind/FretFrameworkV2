#include "GuitarTrack_midi.h"

template <>
void InstrumentalTrack<GuitarNote<5>>::parseSysEx(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str.compare(0, 2, "PS") == 0)
	{
		if (str[4] == (char)0xFF)
		{
			switch (str[5])
			{
			case 1:
				for (size_t diff = 0; diff < 4; ++diff)
					tracker.laneValues[12 * diff + 1] = !str[6];
				break;
			case 4:
				for (size_t diff = 0; diff < 4; ++diff)
				{
					tracker.difficulties[diff].sliderNotes = str[6];
					if (str[6])
					{
						if (auto note = m_difficulties[diff].m_notes.try_back(position))
							note->setTap(true);
					}
				}
				break;
			}
		}
		else
		{
			switch (str[5])
			{
			case 1:
				tracker.laneValues[12 * str[4] + 1] = !str[6];
				break;
			case 4:
				if (str[6])
				{
					tracker.difficulties[str[4]].sliderNotes = true;
					if (auto note = m_difficulties[str[4]].m_notes.try_back(position))
						note->setTap(true);
				}
				else
					tracker.difficulties[str[4]].sliderNotes = false;
				break;
			}
		}
	}
}

template <>
void InstrumentalTrack<GuitarNote<6>>::parseSysEx(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str.compare(0, 2, "PS") == 0 && str[5] == 4)
	{
		if (str[4] == (char)0xFF)
		{
			for (size_t diff = 0; diff < 4; ++diff)
			{
				tracker.difficulties[diff].sliderNotes = str[6];
				if (str[6])
				{
					if (auto note = m_difficulties[diff].m_notes.try_back(position))
						note->setTap(true);
				}
			}
		}
		else if (str[6])
		{
			tracker.difficulties[str[4]].sliderNotes = true;
			if (auto note = m_difficulties[str[4]].m_notes.try_back(position))
				note->setTap(true);
		}
		else
			tracker.difficulties[str[4]].sliderNotes = false;
	}
}

template <>
void InstrumentalTrack<GuitarNote<5>>::parseText(Midi_Tracker& tracker, std::string_view str, uint32_t position)
{
	if (str == "[ENHANCED_OPENS]" || str == "ENHANCED_OPENS")
	{
		for (size_t diff = 0; diff < 4; ++diff)
			tracker.laneValues[12 * diff] = 0;
	}
	else
		m_events.get_or_emplace_back(position).push_back(UnicodeString::strToU32(str));
}

template <>
void InstrumentalTrack<GuitarNote<5>>::writeMidiToggleEvent(MidiFileWriter& writer) const
{
	writer.addText(0, "[ENHANCED_OPENS]");
}

template<size_t numFrets>
void WriteForcingAndTap(MidiFileWriter& writer, const SimpleFlatMap<GuitarNote<numFrets>>& notes, unsigned char diffIndex)
{
	const unsigned char FORCE_OFFSET = 65 + 12 * diffIndex;
	const unsigned char TAP_VALUE = 68 + 12 * diffIndex;
	struct Hold
	{
		uint32_t start = UINT32_MAX;
		uint32_t end = UINT32_MAX;
	};

	struct ForceHold : public Hold
	{
		ForceStatus status = ForceStatus::UNFORCED;
	};

	Hold tap;
	ForceHold forcing;
	for (const auto& node : notes)
	{
		if (node->isTapped())
		{
			if (tap.start != UINT32_MAX)
				tap.start = node.key;
			tap.end = node.key + node->getLongestSustain();
		}
		else if (tap.start != UINT32_MAX)
		{
			if (tap.end > node.key)
				tap.end = node.key;

			writer.addMidiNote(tap.start, TAP_VALUE, 100, tap.end - tap.start);
			tap.start = UINT32_MAX;
		}

		ForceStatus forceStatus = node->getForcing();
		if (forcing.status != forceStatus)
		{
			if (forcing.status == ForceStatus::UNFORCED)
			{
				forcing.status = forceStatus;
				forcing.start = node.key;
				forcing.end = node.key + node->getLongestSustain();
			}
			else
			{
				if (forcing.end > node.key)
					forcing.end = node.key;

				writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.end - forcing.start);

				forcing.status = forceStatus;
				if (forceStatus != ForceStatus::UNFORCED)
				{
					forcing.start = node.key;
					forcing.end = node.key + node->getLongestSustain();
				}
			}
		}
		else if (forcing.status != ForceStatus::UNFORCED)
			forcing.end = node.key + node->getLongestSustain();
	}

	if (tap.start != UINT32_MAX)
		writer.addMidiNote(tap.start, TAP_VALUE, 100, tap.end - tap.start);

	if (forcing.status != ForceStatus::UNFORCED)
		writer.addMidiNote(forcing.start, FORCE_OFFSET + (forcing.status == ForceStatus::HOPO_OFF), 100, forcing.end - forcing.start);
}

template <>
void DifficultyTrack<GuitarNote<5>>::write_details(MidiFileWriter& writer, unsigned char diffIndex) const
{
	WriteForcingAndTap(writer, m_notes, diffIndex);
}

template <>
void DifficultyTrack<GuitarNote<6>>::write_details(MidiFileWriter& writer, unsigned char diffIndex) const
{
	WriteForcingAndTap(writer, m_notes, diffIndex);
}
