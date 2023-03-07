#include "MidiFileReader.h"
#include "VariableLengthQuantity.h"

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

MidiFileReader::MidiFileReader(const std::filesystem::path& path) : BinaryFileReader(path)
{
	if (!checkTag("MThd"))
		throw std::runtime_error("Midi Header Chunk Tag 'MTrk' not found");

	if (extract<uint32_t>() != 6)
		throw std::runtime_error("Midi Header length is invalid (must be six)");

	m_nextTrack = m_currentPosition + 6;

	m_header.format = extract<uint16_t>();
	m_header.numTracks = extract<uint16_t>();
	m_header.tickRate = extract<uint16_t>();
}

bool MidiFileReader::startNextTrack()
{
	if (m_track.count == m_header.numTracks)
		return false;

	m_currentPosition = m_nextTrack;
	m_track.count++;

	if (!checkTag("MTrk"))
		throw std::runtime_error("Midi Track Tag 'MTrk' not found for Track " + std::to_string(m_track.count));

	const uint32_t trackLength = extract<uint32_t>();
	m_nextTrack = m_currentPosition + trackLength;

	if (m_nextTrack > getEndOfFile())
		throw std::runtime_error("Midi Track " + std::to_string(m_track.count) + "'s length extends past End of File");

	m_event.tickPosition = 0;
	m_event.type = MidiEventType::Reset_Or_Meta;

	const char* const ev = m_currentPosition;
	if (parseEvent() && m_event.type == MidiEventType::Text_TrackName)
		m_track.name = extractTextOrSysEx();
	else
	{
		m_currentPosition = ev;
		m_event.tickPosition = 0;
		m_event.type = MidiEventType::Reset_Or_Meta;

		m_track.name = {};
	}
	return true;
}

bool MidiFileReader::parseEvent()
{
	if (m_event.type != MidiEventType::Reset_Or_Meta)
		m_currentPosition += m_event.length;

	m_event.tickPosition += readVLQ();
	unsigned char tmp = *m_currentPosition;
	MidiEventType type = static_cast<MidiEventType>(tmp);
	if (type < MidiEventType::Note_Off)
	{
		if (m_event.type < MidiEventType::Note_Off || m_event.type >= MidiEventType::SysEx)
			throw std::runtime_error("Invalid running event");
	}
	else
	{
		m_currentPosition++;
		if (type < MidiEventType::SysEx)
		{
			m_event.channel = tmp & 15;
			type = static_cast<MidiEventType>(tmp & 240);
			m_event.length = 1 + (type == MidiEventType::Note_Off ||
				                  type == MidiEventType::Note_On ||
				                  type == MidiEventType::Control_Change ||
				                  type == MidiEventType::Key_Pressure ||
				                  type == MidiEventType::Pitch_Wheel);
		}
		else
		{
			switch (type)
			{
			case MidiEventType::Reset_Or_Meta:
				type = static_cast<MidiEventType>(*m_currentPosition++);
				if (type == MidiEventType::End_Of_Track)
					return false;
				__fallthrough;
			case MidiEventType::SysEx:
			case MidiEventType::SysEx_End:
				m_event.length = readVLQ();
				break;
			case MidiEventType::Song_Position:
				m_event.length = 2;
				break;
			case MidiEventType::Song_Select:
				m_event.length = 1;
				break;
			default:
				m_event.length = 0;
			}
		}

		m_event.type = type;
	}

	if (m_currentPosition + m_event.length >= m_nextTrack)
		throw std::runtime_error("Midi Track " + std::to_string(m_track.count) + " ends inproperly");

	return true;
}

std::string_view MidiFileReader::extractTextOrSysEx() const noexcept
{
	return std::string_view(m_currentPosition, m_event.length);
}

MidiNote MidiFileReader::extractMidiNote() const noexcept
{
	MidiNote note;
	memcpy(&note, m_currentPosition, sizeof(MidiNote));
	return note;
}

ControlChange MidiFileReader::extractControlChange() const noexcept
{
	ControlChange control;
	memcpy(&control, m_currentPosition, sizeof(ControlChange));
	return control;
}

uint32_t MidiFileReader::extractMicrosPerQuarter() const noexcept
{
	uint32_t micros = 0;
	memcpy((char*)&micros + 1, m_currentPosition, 3);
	return _byteswap_ulong(micros);
}

TimeSig MidiFileReader::extractTimeSig() const noexcept
{
	TimeSig timeSig;
	memcpy(&timeSig, m_currentPosition, sizeof(TimeSig));
	return timeSig;
}
