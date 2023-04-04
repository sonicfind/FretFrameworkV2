#pragma once
#include "BinaryFileWriter.h"
#include "CommonChartWriter.h"

class BCHFileWriter : private BinaryFileWriter<false>, public CommonChartWriter
{
public:
	using BinaryFileWriter::BinaryFileWriter;

public:
	virtual void setPitchMode(PitchWriteMode mode) override {}

	virtual void writeHeaderTrack(uint32_t tickRate) override;
	virtual void writeSyncTrack() override;
	virtual void writeEventTrack() override;
	virtual void writeNoteTrack(size_t index) override;
	virtual void writeDifficultyTrack(size_t index) override;
	virtual void writeAnimationTrack() override;
	virtual void finishTrack() override;
	virtual void startEvent(uint32_t position, ChartEvent ev) override;
	virtual void finishEvent() override;
	virtual void writeSingleNote(const std::pair<size_t, uint32_t>& note) override;
	virtual void writeMultiNote(const std::vector<std::pair<size_t, uint32_t>>& notes) override;
	virtual void writeSingleNoteMods(const std::vector<char>& mods) override;
	virtual void writeMultiNoteMods(const std::vector<std::pair<char, size_t>>& mods) override;
	virtual void writeText(std::string_view str) override;
	virtual void writeSpecialPhrase(const SpecialPhrase& phrase) override;
	virtual void writePitch(Pitch<-1, 9> pitch) override;

	virtual void writeLyric(std::pair<size_t, std::string_view> lyric) override;
	virtual void writePitchAndDuration(const std::pair<Pitch<-1, 9>, uint32_t>& note) override;

	virtual void writeNoteName(NoteName note, PitchType type) override;
	virtual void writeLeftHand(size_t position) override;

	virtual void writeMicrosPerQuarter(uint32_t micros) override;
	virtual void writeTimeSig(TimeSig timeSig) override;

private:
	using BinaryFileWriter::writeTag;

	std::vector<std::pair<std::streampos, std::streampos>> m_trackPositions;
	uint32_t m_position;
	std::string m_event;
	void writeTrackHeader(const char(&tag)[5]);

	template <size_t OFFSET>
	void writeLengthOfSection()
	{
		const auto curr = tell();
		const uint32_t sectLength = uint32_t(curr - m_trackPositions.back().second);
		seek(m_trackPositions.back().first + std::streamoff(OFFSET));
		write(sectLength);
		seek(curr);
		m_trackPositions.back().second = curr;
	}

	void writeWebType(uint32_t value);

	template <typename T>
	void append(const T* const data, size_t size)
	{
		m_event.append((const char*)data, size);
	}

	template <typename T>
	void append(const T& value, size_t size)
	{
		append(&value, size);
	}

	template <typename T>
	void append(const T& value)
	{
		append(value, sizeof(T));
	}

	void appendWebType(uint32_t value);
};
