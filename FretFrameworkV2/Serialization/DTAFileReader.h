#pragma once
#include "FileReader.h"
#include <functional>

namespace DTA
{
	class DTAFileReader : private FileReader
	{
	public:
		DTAFileReader(const LoadedFile& file);
		std::string_view getNameOfNode();

		template <typename T>
		bool extract(T& value) noexcept
		{
			static_assert(std::is_arithmetic<T>::value);

			if (*m_currentPosition == '+')
				++m_currentPosition;

			auto [ptr, ec] = std::from_chars(m_currentPosition, m_nodeEnds.back(), value);
			if (ec == std::errc::invalid_argument)
				return false;

			m_currentPosition = ptr;
			if (ec != std::errc{})
			{
				if constexpr (std::is_integral<T>::value)
				{
					if constexpr (std::is_signed<T>::value)
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

		std::string_view extractText();

		void runLoop(const std::function<void()>& func);
		
		template <typename T>
		std::vector<T> extractList()
		{
			std::vector<T> values;
			while (*m_currentPosition != ')')
				if constexpr (!std::is_same<T, std::string_view>::value)
					values.push_back(extract<T>());
				else
					values.push_back(extractText());
			return values;
		}

	private:
		bool startNode();
		void endNode();
		void skipWhiteSpace();

	private:
		std::vector<const char*> m_nodeEnds;
	};

	struct DTARanks
	{
		uint16_t guitar5 = 0;
		uint16_t bass5 = 0;
		uint16_t drum4_pro = 0;
		uint16_t keys = 0;
		uint16_t vocals = 0;
		uint16_t real_guitar = 0;
		uint16_t real_bass = 0;
		uint16_t real_keys = 0;
		uint16_t band = 0;
	};

	struct DTAAudio
	{
		float pan[2]{};
		float volume[2]{};
		float core[2] = { -1, -1 };
	};

	class DTAFileNode
	{
	public:
		DTAFileNode(DTAFileReader& reader);
		const DTARanks& getRanks() const noexcept;
		const auto& getOthers() const noexcept { return m_others; }
		
	private:

	private:
		std::string_view m_nodeName;

		std::string_view m_name;
		std::string_view m_artist;
		std::string_view m_album;
		std::string_view m_genre;
		std::string_view m_subgenre;
		std::string_view m_charter;
		std::string_view m_source;
		std::string_view m_songID;
		std::string_view m_encoding;
		std::string_view m_vocalPercBank;
		std::string_view m_drumBank;
		std::string_view m_bandFailCue;
		std::string_view m_decade;
		std::string_view m_fake;
		std::string_view m_downloaded;
		std::string_view m_packname;

		uint32_t m_year_released = UINT32_MAX;
		uint32_t m_year_recorded = UINT32_MAX;
		uint32_t m_albumTrack = UINT32_MAX;
		uint32_t m_scrollSpeed = UINT32_MAX;
		uint32_t m_length = UINT32_MAX;
		uint32_t m_version = UINT32_MAX;
		uint32_t m_format = UINT32_MAX;
		uint32_t m_rating = UINT32_MAX;
		int32_t m_tuningOffsetCents = UINT32_MAX;
		uint32_t m_vocalTonic = UINT32_MAX;
		uint32_t m_shortVersion = 0;
		uint32_t m_animTempo = UINT32_MAX;
		uint32_t m_context = UINT32_MAX;
		uint32_t m_basePoints = UINT32_MAX;

		float m_guidePitchVolume = -1;

		bool m_isMaster = false;
		bool m_hasAlbumArt = false;
		bool m_tonality = false;
		bool m_vocalIsMale = true;

		uint32_t m_preview[2] = { UINT32_MAX, UINT32_MAX };

		DTARanks m_ranks;
		std::vector<std::string_view> m_soloes;
		std::vector<std::string_view> m_videoVenues;

		int16_t m_realGuitarTuning[6]{};
		int16_t m_realBassTuning[4]{};

		std::string_view m_location;
		struct
		{
			std::vector<uint16_t> drum;
			std::vector<uint16_t> bass;
			std::vector<uint16_t> guitar;
			std::vector<uint16_t> keys;
			std::vector<uint16_t> vocals;
			std::vector<uint16_t> crowd;
		} m_trackIndices;
		uint16_t m_numVocalParts = 0;
		struct
		{
			DTAAudio kick;
			DTAAudio snare;
			DTAAudio cymbals;
			DTAAudio bass;
			DTAAudio guitar;
			DTAAudio vocals;
			DTAAudio tracks;
			DTAAudio crowd;
		} m_audio;
		std::string_view m_midiFile;

		std::vector<std::pair<std::string_view, std::string_view>> m_others;
	};
}
