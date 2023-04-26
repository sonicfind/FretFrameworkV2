#pragma once
#include "FileReader.h"
#include "CommonChartParser.h"

using EventCombo = std::pair<std::string_view, ChartEvent>;
class TxtFileReader : public FileReader, public CommonChartParser
{
public:
	enum NoteTracks_V1
	{
		Single,
		DoubleGuitar,
		DoubleBass,
		DoubleRhythm,
		Drums,
		Keys,
		GHLGuitar,
		GHLBass,
		Invalid,
	};
	 
public:
	TxtFileReader(const std::filesystem::path& path);
	TxtFileReader(const TxtFileReader&) = default;
	TxtFileReader(TxtFileReader&&) = default;

	template <typename T>
	bool extract(T& value) noexcept
	{
		static_assert(std::is_arithmetic<T>::value);

		auto [ptr, ec] = std::from_chars(m_currentPosition, m_next, value);
		if (ec == std::errc::invalid_argument)
			return false;

		m_currentPosition = ptr;
		if (ec != std::errc{})
		{
			if constexpr (std::is_integral<T>::value)
			{
				if constexpr (std::is_signed_v<T>)
					value = *m_currentPosition == '-' ? (T)INT64_MIN : (T)INT64_MAX;
				else
					value = (T)UINT64_MAX;
			}

			static constexpr auto testCharacter = [](char ch)
			{
				if constexpr (std::is_floating_point<T>::value)
				{
					if (ch == '.')
						return true;
				}
				return '0' <= ch && ch <= '9';
			};

			while (testCharacter(*m_currentPosition))
				++m_currentPosition;
		}

		skipWhiteSpace();
		return true;
	}

	template <>
	bool extract(char& value);

	template <>
	bool extract(bool& value);

	template <typename T>
	[[nodiscard]] T extract()
	{
		T value;
		if (!extract(value))
			throw std::runtime_error("can not parse this data");

		return value;
	}

public:
	[[nodiscard]] virtual bool isStartOfTrack() override;
	[[nodiscard]] virtual bool validateHeaderTrack() override;
	[[nodiscard]] virtual bool validateSyncTrack() override;
	[[nodiscard]] virtual bool validateEventTrack() override;
	[[nodiscard]] virtual bool validateNoteTrack() override;
	[[nodiscard]] virtual bool validateDifficultyTrack() override;
	[[nodiscard]] virtual bool validateAnimationTrack() override;
	virtual void skipTrack() override;

	[[nodiscard]] virtual bool isStillCurrentTrack() override;
	[[nodiscard]] virtual std::pair<uint32_t, ChartEvent> parseEvent() override;
	virtual void nextEvent() override;

	[[nodiscard]] virtual std::pair<size_t, uint32_t> extractSingleNote() override;
	[[nodiscard]] virtual std::vector<std::pair<size_t, uint32_t>> extractMultiNote() override;
	[[nodiscard]] virtual std::vector<char> extractSingleNoteMods() override;
	[[nodiscard]] virtual std::vector<std::pair<char, size_t>> extractMultiNoteMods() override;
	[[nodiscard]] virtual std::string_view extractText() override;
	[[nodiscard]] virtual SpecialPhrase extractSpecialPhrase() override;

	[[nodiscard]] virtual NoteName extractNoteName() override;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() override;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() override;
	[[nodiscard]] virtual std::pair<Pitch<-1, 9>, uint32_t> extractPitchAndDuration() override;

	[[nodiscard]] virtual size_t extractLeftHand() override;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() override;
	[[nodiscard]] virtual TimeSig extractTimeSig() override;

	[[nodiscard]] std::pair<size_t, uint32_t> extractColorAndSustain_V1();

public:
	std::string_view parseModifierName();
	NoteTracks_V1 extractTrack_V1();

private:
	std::vector<const std::vector<EventCombo>*> m_eventSets;

	void skipWhiteSpace();
	void setNextPointer();
	void gotoNextLine();
	[[nodiscard]] bool validateTrack(std::string_view str);
	[[nodiscard]] bool doesStringMatch(std::string_view str) const;
	[[nodiscard]] bool doesStringMatch_noCase(std::string_view str) const;

	bool checkDifficulty_V1();
	NoteTracks_V1 checkTrack_V1();
};
