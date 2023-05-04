#pragma once
#include "TxtFileWriter.h"
#include "CommonChartWriter.h"

class ChtFileWriter: private TxtFileWriter, public CommonChartWriter
{
public:
	using TxtFileWriter::TxtFileWriter;

public:
	virtual void setPitchMode(PitchType mode) override;

	virtual void writeHeaderTrack(uint32_t tickRate) override;
	virtual void writeSyncTrack() override;
	virtual void writeEventTrack() override;
	virtual void writeNoteTrack(unsigned char index) override;
	virtual void writeDifficultyTrack(unsigned char index) override;
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
	void writeTrack(std::string_view name);
	std::string m_tabs = "";
	PitchType m_pitchMode = PitchType::Sharp;
};
