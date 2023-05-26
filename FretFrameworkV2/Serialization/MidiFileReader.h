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
	[[nodiscard]] std::optional<MidiEvent> parseEvent();

	[[nodiscard]] uint16_t getTickRate() const noexcept { return m_header.tickRate; }
	[[nodiscard]] uint16_t getTrackNumber() const noexcept { return m_trackCount; }
	[[nodiscard]] uint64_t getPosition() const noexcept { return m_event.position; }
	[[nodiscard]] MidiEventType getEventType() const noexcept { return m_event.type; }
	[[nodiscard]] unsigned char getMidiChannel() const noexcept { return m_event.channel; }

	[[nodiscard]] unsigned char getMultiplierNote() const noexcept { return m_multiplierNote; }

	[[nodiscard]] std::string_view extractTextOrSysEx() const noexcept;
	[[nodiscard]] MidiNote extractMidiNote() const noexcept;
	[[nodiscard]] ControlChange extractControlChange() const noexcept;

	[[nodiscard]] uint32_t extractMicrosPerQuarter() const noexcept;
	[[nodiscard]] TimeSig extractTimeSig() const noexcept;

private:
	void processHeaderChunk();
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
	const char* m_nextTrack;
	const char* m_next;
};
