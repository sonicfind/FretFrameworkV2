#include "MidiFileReader.h"

uint32_t MidiFileReader::readVLQ()
{
	size_t index = 0;
	uint32_t value = 0;
	while (m_currentPosition[index] < 0)
	{
		if (index == 3)
			throw std::runtime_error("Invalid variable length quantity");

		value |= m_currentPosition[index++] & 127;
		value <<= 7;
	}
	value |= m_currentPosition[index++] & 127;

	if (m_currentPosition + index >= m_nextTrack)
		throw std::runtime_error("Could not parse variable length quantity");

	m_currentPosition += index;
	return value;
}

void MidiFileReader::processHeaderChunk()
{
	if (!checkTag("MThd"))
		throw std::runtime_error("Midi Header Chunk Tag 'MTrk' not found");

	const uint32_t length = extract<uint32_t>();
	if (length < 6)
		throw std::runtime_error("Midi Header length is invalid (must be at least six)");

	m_nextTrack = m_currentPosition + length;

	m_header.format = extract<uint16_t>();
	m_header.numTracks = extract<uint16_t>();
	m_header.tickRate = extract<uint16_t>();
}

MidiFileReader::MidiFileReader(const std::filesystem::path& path, unsigned char multiplierNote) : BinaryFileReader(path), m_multiplierNote(multiplierNote)
{
	processHeaderChunk();
}

MidiFileReader::MidiFileReader(const LoadedFile& file) : BinaryFileReader(file)
{
	processHeaderChunk();
}

bool MidiFileReader::startTrack()
{
	if (m_trackCount == m_header.numTracks)
		return false;

	m_currentPosition = m_nextTrack;
	m_trackCount++;

	if (!checkTag("MTrk"))
		throw std::runtime_error("Midi Track Tag 'MTrk' not found for Track " + std::to_string(m_trackCount));

	const uint32_t trackLength = extract<uint32_t>();
	m_nextTrack = m_currentPosition + trackLength;

	if (m_nextTrack > m_file.end())
		throw std::runtime_error("Midi Track " + std::to_string(m_trackCount) + "'s length extends past End of File");

	m_event.position = 0;
	m_event.type = MidiEventType::Reset_Or_Meta;

	const char* const ev = m_next = m_currentPosition;
	if (!parseEvent() || m_event.type != MidiEventType::Text_TrackName)
	{
		m_next = ev;
		m_event.position = 0;
		m_event.type = MidiEventType::Reset_Or_Meta;
	}
	return true;
}

std::optional<MidiEvent> MidiFileReader::parseEvent()
{
	static constexpr auto getOffset = [](MidiEventType type)
	{
		return 1 + (type == MidiEventType::Note_On ||
				    type == MidiEventType::Note_Off ||
				    type == MidiEventType::Control_Change ||
				    type == MidiEventType::Key_Pressure ||
				    type == MidiEventType::Pitch_Wheel);
	};

	m_currentPosition = m_next;

	m_event.position += readVLQ();
	unsigned char tmp = *m_currentPosition;
	MidiEventType type = static_cast<MidiEventType>(tmp);
	if (type < MidiEventType::Note_Off)
	{
		if (m_event.type < MidiEventType::Note_Off || m_event.type >= MidiEventType::SysEx)
			throw std::runtime_error("Invalid running event");
		m_next = m_currentPosition + getOffset(m_event.type);
	}
	else
	{
		m_currentPosition++;
		if (type < MidiEventType::SysEx)
		{
			m_event.channel = tmp & 15;
			type = static_cast<MidiEventType>(tmp & 240);
			m_next = m_currentPosition + getOffset(type);
		}
		else
		{
			switch (type)
			{
			case MidiEventType::Reset_Or_Meta:
				type = static_cast<MidiEventType>(*m_currentPosition++);
				if (type == MidiEventType::End_Of_Track)
					return {};
				__fallthrough;
			case MidiEventType::SysEx:
			case MidiEventType::SysEx_End:
			{
				uint32_t length = readVLQ();
				m_next = m_currentPosition + length;
				break;
			}
			case MidiEventType::Song_Position:
				m_next = m_currentPosition + 2;
				break;
			case MidiEventType::Song_Select:
				m_next = m_currentPosition + 1;
				break;
			}
		}

		m_event.type = type;
	}

	if (m_next >= m_nextTrack)
		throw std::runtime_error("Midi Track " + std::to_string(m_trackCount) + " ends improperly");

	return m_event;
}

std::string_view MidiFileReader::extractTextOrSysEx() const noexcept
{
	return std::string_view(m_currentPosition, m_next);
}

MidiNote MidiFileReader::extractMidiNote() const noexcept
{
	return { (unsigned char)m_currentPosition[0], (unsigned char)m_currentPosition[1] };
}

ControlChange MidiFileReader::extractControlChange() const noexcept
{
	return { (unsigned char)m_currentPosition[0], (unsigned char)m_currentPosition[1] };
}

uint32_t MidiFileReader::extractMicrosPerQuarter() const noexcept
{
	uint32_t micros = 0;
	memcpy((char*)&micros + 1, m_currentPosition, 3);
	return _byteswap_ulong(micros);
}

TimeSig MidiFileReader::extractTimeSig() const noexcept
{
	return { (unsigned char)m_currentPosition[0],
		     (unsigned char)m_currentPosition[1],
		     (unsigned char)m_currentPosition[2],
		     (unsigned char)m_currentPosition[3] };
}
