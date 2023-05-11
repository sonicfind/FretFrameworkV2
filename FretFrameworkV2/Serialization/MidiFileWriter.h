#pragma once
#include "BinaryFileWriter.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Song/TimeSig.h"
#include "MidiStructs.h"

class MidiFileWriter : private BinaryFileWriter<true>
{
	struct Sysex
	{
		static char BUFFER[8];

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

	void addMidiNote(uint64_t position, unsigned char value, unsigned char velocity, uint64_t length, unsigned char channel = 0);
	void addSysex(uint64_t position, unsigned char diff, unsigned char type, uint64_t length);
	void addText(uint64_t position, std::string&& str, MidiEventType type = MidiEventType::Text);
	void addMicros(uint64_t position, uint32_t micros);
	void addTimeSig(uint64_t position, TimeSig sig);
	void writeTrack();

private:
	void writeVLQ(uint32_t value);
	void writeMeta(MidiEventType type, std::string_view str);
	void writeString(MidiEventType type, std::string_view str);

private:
	struct
	{
		uint16_t format = 1;
		uint16_t numTracks;
		uint16_t tickRate;
	} m_header;

	std::string m_trackname;
	SimpleFlatMap<MidiWriteNode> m_nodes;
};
