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

		char diff;
		char type;
		char status;
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

	template <bool isON>
	void addMidiNote(uint32_t position, MidiNote note, char channel = 0)
	{
		if constexpr (isON)
			m_nodes[position].noteOns.push_back({ channel, note });
		else
			m_nodes[position].noteOffs.push_back({ channel, note });
	}

	template <bool isON>
	void addSysex(uint32_t position, char diff, char type, char status)
	{
		if constexpr (isON)
			m_nodes[position].sysexOns.push_back({ diff, type, status });
		else
			m_nodes[position].sysexOffs.push_back({ diff, type, status });
	}

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
