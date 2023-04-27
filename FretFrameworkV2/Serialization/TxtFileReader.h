#pragma once
#include "FileReader.h"

class TxtFileReader : public FileReader
{
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

	std::string_view extractText();

	void skipTrack();

public:
	std::string_view parseModifierName();

protected:
	void skipWhiteSpace();
	void setNextPointer();
	void gotoNextLine();
	[[nodiscard]] bool doesStringMatch(std::string_view str) const;
	[[nodiscard]] bool doesStringMatch_noCase(std::string_view str) const;
};
