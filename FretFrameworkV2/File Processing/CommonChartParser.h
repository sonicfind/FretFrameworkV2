#pragma once
#include "CommonChartTypes.h"

class CommonChartParser
{
protected:
	uint32_t m_tickPosition = 0;
	size_t m_instrumentTrackID = 0;
	size_t m_vocalTrackID = 0;
	size_t m_difficulty = 0;

public:
	[[nodiscard]] virtual bool isStartOfTrack() = 0;
	[[nodiscard]] virtual bool validateHeaderTrack() = 0;
	[[nodiscard]] virtual bool validateSyncTrack() = 0;
	[[nodiscard]] virtual bool validateEventTrack() = 0;
	[[nodiscard]] virtual bool validateInstrumentTrack() = 0;
	[[nodiscard]] virtual bool validateVocalTrack() = 0;
	[[nodiscard]] virtual bool validateDifficultyTrack() = 0;
	[[nodiscard]] virtual bool validateAnimationTrack() = 0;
	virtual void processUnknownTrack() = 0;
	virtual void skipTrack() = 0;
	virtual void endTrack() = 0;

	[[nodiscard]] virtual bool isStillCurrentTrack() const = 0;
	[[nodiscard]] virtual uint32_t parsePosition() = 0;
	[[nodiscard]] virtual ChartEvent parseEvent() = 0;
	virtual void nextEvent() = 0;

	[[nodiscard]] virtual std::pair<size_t, uint32_t> extractSingleNote() = 0;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint32_t>> extractMultiNote() = 0;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() = 0;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() = 0;
	[[nodiscard]] virtual std::string_view extractText() = 0;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() = 0;
	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() = 0;
	[[nodiscard]] virtual std::pair<char, uint32_t> extractPitchAndDuration() = 0;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() = 0;
	[[nodiscard]] virtual TimeSig extractTimeSig() = 0;
	[[nodiscard]] virtual uint32_t extractAnchor() = 0;

	[[nodiscard]] size_t getInstrumentTrackID() const noexcept { return m_instrumentTrackID; }
	[[nodiscard]] size_t getVocalTrackID() const noexcept { return m_vocalTrackID; }
	[[nodiscard]] size_t getDifficulty() const noexcept { return m_difficulty; }
};
