#pragma once
#include "BinaryFileReader.h"
#include "CommonChartParser.h"

class BCHFileReader : public BinaryFileReader<false>, public CommonChartParser
{
public:
	using BinaryFileReader::BinaryFileReader;

	template <typename T>
	[[nodiscard]] bool testExtract() const noexcept
	{
		return move(sizeof(T));
	}

	template <bool useVirtual = true>
	bool extractWebType(uint32_t& value)
	{
		value = 0;
		if (!extract<uint32_t, useVirtual>(value, 1))
			return false;

		return value < 254 || extract<uint32_t, useVirtual>(value, 2 + 2ULL * (value == 255));
	}

	template <bool useVirtual = true>
	[[nodiscard]] uint32_t extractWebType()
	{
		uint32_t value;
		if (!extractWebType<useVirtual>(value))
			throw std::runtime_error("can not parse this data");
		return value;
	}
	[[nodiscard]] bool testExtractWebType() noexcept;

	virtual bool move(size_t amount) override;

	[[nodiscard]] virtual bool isStartOfTrack() override;
	[[nodiscard]] virtual bool validateHeaderTrack() override;
	[[nodiscard]] virtual bool validateSyncTrack() override;
	[[nodiscard]] virtual bool validateEventTrack() override;
	[[nodiscard]] virtual bool validateNoteTrack() override;
	[[nodiscard]] virtual bool validateDifficultyTrack() override;
	[[nodiscard]] virtual bool validateAnimationTrack() override;
	virtual void skipUnknownTrack() override;

	[[nodiscard]] virtual bool isStillCurrentTrack() override;
	[[nodiscard]] virtual uint32_t parsePosition() override;
	[[nodiscard]] virtual ChartEvent parseEvent() override;
	 virtual void nextEvent() override;

	[[nodiscard]] virtual std::pair<size_t, uint32_t> extractSingleNote() override;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint32_t>> extractMultiNote() override;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() override;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() override;
	[[nodiscard]] virtual std::string_view extractText() override;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() override;

	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() override;
	[[nodiscard]] virtual NoteName extractNoteName() override;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() override;
	[[nodiscard]] virtual std::pair<Pitch<-1, 9>, uint32_t> extractPitchAndDuration() override;

	[[nodiscard]] virtual size_t extractLeftHand() override;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() override;
	[[nodiscard]] virtual TimeSig extractTimeSig() override;

private:
	std::vector<const char*> m_nextTracks;

	using BinaryFileReader::checkTag;
	void parseTrackHeader();
	
	[[nodiscard]] bool validateTrack(const char(&str)[5]);
	[[nodiscard]] std::string_view extractText(size_t length);
};
