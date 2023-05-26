#include "MidiFileWriter.h"

MidiFileWriter::MidiFileWriter(const std::filesystem::path& path, uint16_t tickRate) : BinaryFileWriter(path)
{
	m_header.tickRate = tickRate;
	write("MThd", 4);
	write(_byteswap_ulong(6));
	write(m_header);
}

MidiFileWriter::~MidiFileWriter()
{
	seek(8);
	write(_byteswap_ushort(m_header.format));
	write(_byteswap_ushort(m_header.numTracks));
	write(_byteswap_ushort(m_header.tickRate));
}

void MidiFileWriter::startTrack(std::string_view str)
{
	write("MTrk", 4);
	write(uint32_t());
	m_trackPosition = tell();
	m_event.position = 0;
	m_event.channel = 0;
	m_event.type = MidiEventType::Reset_Or_Meta;
	m_header.numTracks++;

	if (!str.empty())
	{
		write(char(0));
		writeMeta(MidiEventType::Text_TrackName, str);
	}
}

void MidiFileWriter::writeText(uint64_t position, const std::string& str, MidiEventType type)
{
	writeVLQ(uint32_t(position - m_event.position));
	writeMeta(type, str);
	m_event.position = position;
}

void MidiFileWriter::writeSysex(uint64_t position, Sysex sysex, bool status)
{
	static char BUFFER[8] = { 'P', 'S', 0, 0, 0, 0, 0, (char)0xF7 };
	BUFFER[4] = (char)sysex.diff;
	BUFFER[5] = (char)sysex.type;
	BUFFER[6] = (char)status;

	writeVLQ(uint32_t(position - m_event.position));
	writeString(MidiEventType::SysEx, { BUFFER, 8 });
	m_event.position = position;
}

void MidiFileWriter::writeMidiNote(uint64_t position, NoteNode note)
{
	writeVLQ(uint32_t(position - m_event.position));

	if (m_event.type != MidiEventType::Note_On || m_event.channel != note.channel)
		write(char((char)MidiEventType::Note_On | note.channel));
	write(note.note);
	write(note.velocity);

	m_event.position = position;
	m_event.type = MidiEventType::Note_On;
	m_event.channel = note.channel;
}

void MidiFileWriter::writeMicros(uint64_t position, uint32_t micros)
{
	static char BUFFER[3];
	if (micros >= 1 << 24)
		throw std::runtime_error("MicroSeconds value exceeds 24bit max");

	BUFFER[0] = (micros >> 16) & 0xFF;
	BUFFER[1] = (micros >> 8) & 0xFF;
	BUFFER[2] = micros & 0xFF;

	writeVLQ(uint32_t(position - m_event.position));
	writeMeta(MidiEventType::Tempo, { BUFFER, 3 });
	m_event.position = position;
}

void MidiFileWriter::writeTimeSig(uint64_t position, TimeSig sig)
{
	static char BUFFER[4];
	memcpy(BUFFER, &sig, 4);

	writeVLQ(uint32_t(position - m_event.position));
	writeMeta(MidiEventType::Time_Sig, { BUFFER, 4 });
	m_event.position = position;
}

void MidiFileWriter::finishTrack()
{
	write(char(0));
	writeMeta(MidiEventType::End_Of_Track, {});

	auto end = tell();
	seek(m_trackPosition - std::streamoff(4));
	write(_byteswap_ulong(uint32_t(end - m_trackPosition)));
	seek(end);
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
				buffer[j] |= (char)0x80;
			write<char>(buffer + i, 4 - i);
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
	m_event.type = type;
}
