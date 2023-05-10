#pragma once
#include "BufferedBinaryReader.h"
#include "CommonChartParser.h"

class BCHFileReader : public BufferedBinaryReader, public CommonChartParser
{
public:
	using BufferedBinaryReader::BufferedBinaryReader;

	[[nodiscard]] virtual bool isStartOfTrack() const override;
	[[nodiscard]] virtual bool validateHeaderTrack() override;
	[[nodiscard]] virtual bool validateSyncTrack() override;
	[[nodiscard]] virtual bool validateEventTrack() override;
	[[nodiscard]] virtual bool validateNoteTrack() override;
	[[nodiscard]] virtual bool validateDifficultyTrack() override;
	[[nodiscard]] virtual bool validateAnimationTrack() override;
	virtual void skipTrack() override;

	[[nodiscard]] virtual bool isStillCurrentTrack() override;
	[[nodiscard]] virtual std::pair<uint64_t, ChartEvent> parseEvent() override;
	 virtual void nextEvent() override;

	[[nodiscard]] virtual std::pair<size_t, uint64_t> extractSingleNote() override;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint64_t>> extractMultiNote() override;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() override;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() override;
	[[nodiscard]] virtual std::string_view extractText() override;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() override;

	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() override;
	[[nodiscard]] virtual NoteName extractNoteName() override;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() override;
	[[nodiscard]] virtual std::pair<Pitch<-1, 9>, uint64_t> extractPitchAndDuration() override;

	[[nodiscard]] virtual size_t extractLeftHand() override;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() override;
	[[nodiscard]] virtual TimeSig extractTimeSig() override;

private:
	std::vector<const char*> m_nextTracks;

	using BinaryFileReader::checkTag;
	void parseTrackHeader();
	
	[[nodiscard]] bool validateTrack(const char(&str)[5]);
};
