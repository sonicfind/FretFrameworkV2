#pragma once
#include "BinaryFileWriter.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Song/TimeSig.h"
#include "MidiStructs.h"

class MidiFileWriter : private BinaryFileWriter<true>
{
	struct Sysex
	{
		static char BUFFER[10];

		unsigned char diff;
		unsigned char type;
		unsigned char status;
		void set() const;
	};

	struct MidiWriteNode
	{
		std::vector<std::pair<char, MidiNote>> noteOffs;
		std::vector<Sysex> sysexOffs;
		std::vector<std::pair<MidiEventType, std::string>> events;
		std::vector<Sysex> sysexOns;
		std::vector<std::pair<char, MidiNote>> noteOns;
	};
public:
	MidiFileWriter(const std::filesystem::path& path, uint16_t tickRate);
	~MidiFileWriter();
	void setTrackName(std::string_view str);

	void addMidiNote(uint32_t position, unsigned char value, unsigned char velocity, uint32_t length, unsigned char channel = 0);
	void addSysex(uint32_t position, unsigned char diff, unsigned char type, uint32_t length);
	void addText(uint32_t position, std::string&& str, MidiEventType type = MidiEventType::Text);
	void addMicros(uint32_t position, uint32_t micros);
	void addTimeSig(uint32_t position, TimeSig sig);
	void writeTrack();

private:
	void writeVLQ(uint32_t value);
	void writeMeta(MidiEventType type, std::string_view str);
	void writeString(MidiEventType type, std::string_view str);

private:
	struct
	{
		uint16_t format;
		uint16_t numTracks;
		uint16_t tickRate;
	} m_header;

	std::string_view m_trackname;
	SimpleFlatMap<MidiWriteNode> m_nodes;
};
