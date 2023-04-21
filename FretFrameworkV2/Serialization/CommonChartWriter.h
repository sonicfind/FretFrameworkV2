#pragma once
#include "CommonChartTypes.h"
enum class PitchWriteMode
{
	Sharp,
	Flat
};

class CommonChartWriter
{
public:
	virtual void setPitchMode(PitchWriteMode mode) = 0;

	virtual void writeHeaderTrack(uint32_t tickRate) = 0;
	virtual void writeSyncTrack() = 0;
	virtual void writeEventTrack() = 0;
	virtual void writeNoteTrack(size_t index) = 0;
	virtual void writeDifficultyTrack(size_t index) = 0;
	virtual void writeAnimationTrack() = 0;
	virtual void finishTrack() = 0;
	virtual void startEvent(uint32_t position, ChartEvent ev) = 0;
	virtual void finishEvent() = 0;
	virtual void writeSingleNote(const std::pair<size_t, uint32_t>& note) = 0;
	virtual void writeMultiNote(const std::vector<std::pair<size_t, uint32_t>>& notes) = 0;
	virtual void writeSingleNoteMods(const std::vector<char>& mods) = 0;
	virtual void writeMultiNoteMods(const std::vector<std::pair<char, size_t>>& mods) = 0;
	virtual void writeText(std::string_view str) = 0;
	virtual void writeSpecialPhrase(const SpecialPhrase& phrase) = 0;
	virtual void writePitch(Pitch<-1, 9> pitch) = 0;

	virtual void writeLyric(std::pair<size_t, std::string_view> lyric) = 0;
	virtual void writePitchAndDuration(const std::pair<Pitch<-1, 9>, uint32_t>& note) = 0;

	virtual void writeNoteName(NoteName note, PitchType type) = 0;
	virtual void writeLeftHand(size_t position) = 0;

	virtual void writeMicrosPerQuarter(uint32_t micros) = 0;
	virtual void writeTimeSig(TimeSig timeSig) = 0;
};
