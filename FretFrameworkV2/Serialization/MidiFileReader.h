#pragma once
#include "BinaryFileReader.h"
#include "Song/TimeSig.h"
#include "MidiStructs.h"
#include <optional>

class MidiFileReader : private BinaryFileReader
{
public:
	MidiFileReader(const std::filesystem::path& path, unsigned char multiplierNote);
	MidiFileReader(const LoadedFile& file);

	[[nodiscard]] bool startTrack();
	[[nodiscard]] bool tryParseEvent();
	[[nodiscard]] MidiEvent getEvent() { return m_event; }

	[[nodiscard]] uint16_t getTickRate() const noexcept { return m_header.tickRate; }
	[[nodiscard]] uint16_t getTrackNumber() const noexcept { return m_trackCount; }

	[[nodiscard]] unsigned char getMultiplierNote() const noexcept { return m_multiplierNote; }

	[[nodiscard]] std::string extractTextOrSysEx() noexcept;
	[[nodiscard]] MidiNote extractMidiNote();
	[[nodiscard]] ControlChange extractControlChange();

	[[nodiscard]] uint32_t extractMicrosPerQuarter();
	[[nodiscard]] TimeSig extractTimeSig();

private:
	void processHeaderChunk();
	uint32_t getEventOffset() const noexcept;
	[[nodiscard]] uint32_t readVLQ();

private:
	struct
	{
		uint16_t format;
		uint16_t numTracks;
		uint16_t tickRate;
	} m_header;

	uint16_t m_trackCount = 0;

	MidiEvent m_event;

	const unsigned char m_multiplierNote = 116;
};
