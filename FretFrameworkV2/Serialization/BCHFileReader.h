#pragma once
#include "BufferedBinaryReader.h"
#include "CommonChartParser.h"

class BCHFileReader : private BufferedBinaryReader, public CommonChartParser
{
public:
	BCHFileReader(const std::filesystem::path& path);
	BCHFileReader(const LoadedFile& file);

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
	virtual void skipEvent() override;
	virtual void nextEvent() override;

	[[nodiscard]] virtual std::pair<size_t, uint64_t> extractSingleNote() override;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint64_t>> extractMultiNote() override;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() override;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() override;
	[[nodiscard]] virtual std::string_view extractText() override;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() override;

	[[nodiscard]] virtual NoteName extractNoteName() override;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() override;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitchAndDuration() override;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() override;

	[[nodiscard]] virtual size_t extractLeftHand() override;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() override;
	[[nodiscard]] virtual uint64_t extractAnchor() override;
	[[nodiscard]] virtual TimeSig extractTimeSig() override;

	uint32_t getTickRate() const noexcept { return m_tickrate; };

private:
	std::vector<const char*> m_nextTracks;
	uint32_t m_tickrate;
	
	[[nodiscard]] bool validateTrack(const char(&str)[5]);
	void parseHeader();
};
