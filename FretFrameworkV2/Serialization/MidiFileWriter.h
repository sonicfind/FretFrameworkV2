#pragma once
#include "BinaryFileWriter.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
#include "Song/TimeSig.h"
#include "MidiStructs.h"

class MidiFileWriter : private BinaryFileWriter<true>
{
public:
	struct Sysex
	{
		unsigned char diff;
		unsigned char type;
	};

	struct NoteNode
	{
		unsigned char channel;
		unsigned char note;
		unsigned char velocity;
	};

	using PhraseList = SimpleFlatMap<std::vector<std::pair<unsigned char, uint64_t>>>;
	using SysexList = SimpleFlatMap<std::vector<std::pair<Sysex, uint64_t>>>;
	using MidiNoteList = SimpleFlatMap<std::vector<std::pair<NoteNode, uint64_t>>>;

public:
	MidiFileWriter(const std::filesystem::path& path, uint16_t tickRate);
	~MidiFileWriter();
	void startTrack(std::string_view str);
	void writeText(uint64_t position, const std::string& str, MidiEventType type = MidiEventType::Text);
	void writeSysex(uint64_t position, Sysex sysex, bool status);
	void writeMidiNote(uint64_t position, NoteNode note);
	
	void writeMicros(uint64_t position, uint32_t micros);
	void writeTimeSig(uint64_t position, TimeSig sig);
	void finishTrack();

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

	MidiEvent m_event;
	std::streampos m_trackPosition;
};
