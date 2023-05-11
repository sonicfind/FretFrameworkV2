#pragma once
#include "TxtFileReader.h"
#include "CommonChartParser.h"

using EventCombo = std::pair<std::string_view, ChartEvent>;
class ChtFileReader : private TxtFileReader, public CommonChartParser
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
	using TxtFileReader::TxtFileReader;

private:
	[[nodiscard]] bool doesStringMatch(std::string_view str) const;

public:
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

	[[nodiscard]] virtual NoteName extractNoteName() override;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitch() override;
	[[nodiscard]] virtual Pitch<-1, 9> extractPitchAndDuration() override;

	[[nodiscard]] virtual std::pair<size_t, std::string_view> extractLyric() override;

	[[nodiscard]] virtual size_t extractLeftHand() override;

	[[nodiscard]] virtual uint32_t extractMicrosPerQuarter() override;
	[[nodiscard]] virtual uint64_t extractAnchor() override;
	[[nodiscard]] virtual TimeSig extractTimeSig() override;

	[[nodiscard]] std::pair<size_t, uint64_t> extractColorAndSustain_V1();

public:
	NoteTracks_V1 extractTrackType_V1();
	[[nodiscard]] std::optional<std::pair<uint64_t, ChartEvent>> extractEvent_V1();

private:
	std::vector<const std::vector<EventCombo>*> m_eventSets;

	[[nodiscard]] bool validateTrack(std::string_view str);

	bool checkDifficulty_V1();
	NoteTracks_V1 checkTrack_V1();

public:
	std::vector<Modifiers::Modifier> extractModifiers(const ModifierOutline& list);
};
