#pragma once
#include "CommonChartTypes.h"

class CommonChartParser
{
protected:
	uint64_t m_tickPosition = 0;
	size_t m_noteTrackID = 0;
	size_t m_difficulty = 0;

public:
	[[nodiscard]] virtual bool isStartOfTrack() const = 0;
	[[nodiscard]] virtual bool validateHeaderTrack() = 0;
	[[nodiscard]] virtual bool validateSyncTrack() = 0;
	[[nodiscard]] virtual bool validateEventTrack() = 0;
	[[nodiscard]] virtual bool validateNoteTrack() = 0;
	[[nodiscard]] virtual bool validateDifficultyTrack() = 0;
	[[nodiscard]] virtual bool validateAnimationTrack() = 0;
	virtual void skipTrack() = 0;

	[[nodiscard]] virtual bool isStillCurrentTrack() = 0;
	[[nodiscard]] virtual std::pair<uint64_t, ChartEvent> parseEvent() = 0;
	virtual void nextEvent() = 0;

	[[nodiscard]] virtual std::pair<size_t, uint64_t> extractSingleNote() = 0;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint64_t>> extractMultiNote() = 0;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() = 0;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() = 0;
	[[nodiscard]] virtual std::string_view extractText() = 0;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() = 0;

	[[nodiscard]] virtual NoteName extractNoteName() = 0;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() = 0;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() = 0;
	[[nodiscard]] virtual std::pair<Pitch<-1, 9>, uint64_t> extractPitchAndDuration() = 0;

	[[nodiscard]] virtual size_t extractLeftHand() = 0;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() = 0;
	[[nodiscard]] virtual TimeSig extractTimeSig() = 0;

	[[nodiscard]] size_t geNoteTrackID() const noexcept { return m_noteTrackID; }
	[[nodiscard]] size_t getDifficulty() const noexcept { return m_difficulty; }
};
