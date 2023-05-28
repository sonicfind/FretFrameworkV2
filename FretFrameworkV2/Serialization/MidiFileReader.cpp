#include "MidiFileReader.h"

uint32_t MidiFileReader::readVLQ()
{
	size_t index = 0;
	uint32_t value = 0;
	while (true)
	{
		char c = extract<char>();
		value |= c & 127;
		if (c >= 0)
			return value;

		if (index == 3)
			throw std::runtime_error("Invalid variable length quantity");

		++index;
		value <<= 7;
	}
}

void MidiFileReader::processHeaderChunk()
{
	if (!checkTag("MThd"))
		throw std::runtime_error("Midi Header Chunk Tag 'MTrk' not found");

	enterSection(_byteswap_ulong(extract<uint32_t>()));
	m_header.format = _byteswap_ushort(extract<uint16_t>());
	m_header.numTracks = _byteswap_ushort(extract<uint16_t>());
	m_header.tickRate = _byteswap_ushort(extract<uint16_t>());
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

	if (m_event.type != MidiEventType::Reset_Or_Meta)
		exitSection();
	exitSection();
	m_trackCount++;

	if (!checkTag("MTrk"))
		throw std::runtime_error("Midi Track Tag 'MTrk' not found for Track " + std::to_string(m_trackCount));

	enterSection(_byteswap_ulong(extract<uint32_t>()));

	m_event.position = 0;
	m_event.type = MidiEventType::Reset_Or_Meta;

	const char* const start = m_currentPosition;
	if (!tryParseEvent() || m_event.type != MidiEventType::Text_TrackName)
	{
		exitSection();
		m_event.type = MidiEventType::Reset_Or_Meta;
		m_currentPosition = start;
		m_event.position = 0;
	}
	return true;
}

uint32_t MidiFileReader::getEventOffset() const noexcept
{
	return 1 + (m_event.type == MidiEventType::Note_On ||
				m_event.type == MidiEventType::Note_Off ||
				m_event.type == MidiEventType::Control_Change ||
				m_event.type == MidiEventType::Key_Pressure ||
				m_event.type == MidiEventType::Pitch_Wheel);
}

bool MidiFileReader::tryParseEvent()
{
	if (m_event.type != MidiEventType::Reset_Or_Meta)
		exitSection();

	m_event.position += readVLQ();
	unsigned char tmp = *m_currentPosition;
	MidiEventType type = static_cast<MidiEventType>(tmp);
	if (type < MidiEventType::Note_Off)
	{
		if (m_event.type < MidiEventType::Note_Off || m_event.type >= MidiEventType::SysEx)
			throw std::runtime_error("Invalid running event");
		enterSection(getEventOffset());
	}
	else
	{
		move(1);
		if (type < MidiEventType::SysEx)
		{
			m_event.channel = tmp & 15;
			m_event.type = static_cast<MidiEventType>(tmp & 240);
			enterSection(getEventOffset());
		}
		else
		{
			switch (type)
			{
			case MidiEventType::Reset_Or_Meta:
				type = static_cast<MidiEventType>(extract<unsigned char>());
				__fallthrough;
			case MidiEventType::SysEx:
			case MidiEventType::SysEx_End:
			{
				enterSection(readVLQ());
				break;
			}
			case MidiEventType::Song_Position:
				enterSection(2);
				break;
			case MidiEventType::Song_Select:
				enterSection(1);
				break;
			default:
				enterSection(0);
			}
			m_event.type = type;

			if (m_event.type == MidiEventType::End_Of_Track)
				return false;
		}
	}
	return true;
}

std::string MidiFileReader::extractTextOrSysEx() noexcept
{
	std::string str(m_currentPosition, m_ends.back());
	m_currentPosition = m_ends.back();
	return str;
}

MidiNote MidiFileReader::extractMidiNote()
{
	return extract<MidiNote>();
}

ControlChange MidiFileReader::extractControlChange()
{
	return extract<ControlChange>();
}

uint32_t MidiFileReader::extractMicrosPerQuarter()
{
	unsigned char buf[3];
	extract(buf);
	return (buf[0] << 16) | (buf[1] << 8) | buf[0];
}

TimeSig MidiFileReader::extractTimeSig()
{
	return extract<TimeSig>();
}
