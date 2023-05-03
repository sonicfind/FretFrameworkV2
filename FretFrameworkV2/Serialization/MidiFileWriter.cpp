#include "MidiFileWriter.h"

MidiFileWriter::MidiFileWriter(const std::filesystem::path& path, uint16_t tickRate) : BinaryFileWriter(path)
{
	m_header.tickRate = tickRate;
	writeTag("MThd");
	write(uint32_t(6));
	write(m_header);
}

MidiFileWriter::~MidiFileWriter()
{
	seek(8);
	write(m_header);
}

void MidiFileWriter::setTrackName(std::string_view str)
{
	m_trackname = str;
}

void MidiFileWriter::addText(uint32_t position, std::string&& str, MidiEventType type)
{
	m_nodes[position].events.push_back({ type, std::move(str) });
}

void MidiFileWriter::addMicros(uint32_t position, uint32_t micros)
{
	static char BUFFER[3];
	if (micros >= 1 << 24)
		throw std::runtime_error("MicroSeconds value exceeds 24bit max");

	micros = _byteswap_ulong(micros);
	memcpy(BUFFER, (char*)&micros + 1, 3);
	m_nodes[position].events.push_back({ MidiEventType::Tempo, { BUFFER, 3 } });
}

void MidiFileWriter::addTimeSig(uint32_t position, TimeSig sig)
{
	static char BUFFER[4];
	memcpy(BUFFER, &sig, 4);
	m_nodes[position].events.push_back({ MidiEventType::Time_Sig, { BUFFER, 4 } });
}

char MidiFileWriter::Sysex::BUFFER[10] = { (char)0xF0, 8, 'P', 'S', 0, 0, 0, 0, 0, (char)0xF7 };
void MidiFileWriter::writeTrack()
{
	writeTag("MTrk");
	write(uint32_t());
	auto start = tell();

	if (!m_trackname.empty())
	{
		write(char(0));
		writeMeta(MidiEventType::Text_TrackName, m_trackname);
	}

	uint32_t position = 0;
	MidiEventType currEvent = MidiEventType::Reset_Or_Meta;
	char currChannel = 0;
	for (const auto& node : m_nodes)
	{
		uint32_t delta = node.key - position;
		for (const std::pair<char, MidiNote>& off : node->noteOffs)
		{
			writeVLQ(delta);
			if (currEvent != MidiEventType::Note_On || currChannel != off.first)
				write((char)MidiEventType::Note_On | off.first);
			write(off.second);

			delta = 0;
			currEvent = MidiEventType::Note_On;
			currChannel = off.first;
		}

		for (const Sysex sysex : node->sysexOffs)
		{
			writeVLQ(delta);
			sysex.set();
			write(Sysex::BUFFER);

			delta = 0;
			currEvent = MidiEventType::SysEx;
		}

		for (const auto& ev : node->events)
		{
			writeVLQ(delta);
			writeMeta(ev.first, ev.second);

			delta = 0;
			currEvent = ev.first;
		}

		for (const Sysex sysex : node->sysexOns)
		{
			writeVLQ(delta);
			sysex.set();
			write(Sysex::BUFFER);

			delta = 0;
			currEvent = MidiEventType::SysEx;
		}

		for (const std::pair<char, MidiNote>& on : node->noteOffs)
		{
			writeVLQ(delta);
			if (currEvent != MidiEventType::Note_On || currChannel != on.first)
				write((char)MidiEventType::Note_On | on.first);
			write(on.second);

			delta = 0;
			currEvent = MidiEventType::Note_On;
			currChannel = on.first;
		}
		position = node.key;
	}

	write(char(0));
	writeMeta(MidiEventType::End_Of_Track, {});

	auto end = tell();
	seek(start - std::streamoff(4));
	write(uint32_t(end - start));
	seek(end);

	m_header.numTracks++;
}

void MidiFileWriter::writeVLQ(uint32_t value)
{
	if (value > 0x0FFFFFFF)
		throw std::runtime_error("VLQ value exceeds 28bit max");
	
	if (value == 0)
	{
		write(char(0));
		return;
	}

	char buffer[4] = { char(value >> 21), char((value >> 14) & 0x7F), char((value >> 7) & 0x7F), char(value & 0x7F) };
	for (size_t i = 0; i < 4; ++i)
	{
		if (buffer[i] > 0)
		{
			for (size_t j = i; j < 3; ++j)
				buffer[j] |= 0x80;
			write(buffer + i, 4 - i);
			return;
		}
	}
}

void MidiFileWriter::writeMeta(MidiEventType type, std::string_view str)
{
	write((char)0xFF);
	writeString(type, str);
}

void MidiFileWriter::writeString(MidiEventType type, std::string_view str)
{
	write((char)type);
	writeVLQ((uint32_t)str.size());
	write(str.data(), str.size());
}

void MidiFileWriter::Sysex::set() const
{
	Sysex::BUFFER[6] = diff;
	Sysex::BUFFER[7] = type;
	Sysex::BUFFER[8] = status;
}
