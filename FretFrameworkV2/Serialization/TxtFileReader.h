#pragma once
#include "FileReader.h"
#include "Modifiers.h"
#include <vector>

class TxtFileReader : public FileReader
{
public:
	TxtFileReader(const std::filesystem::path& path);
	TxtFileReader(const LoadedFile& file);

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

	std::string_view extractText(bool checkForQuotes = true);

protected:
	void gotoNextLine();
	void skipWhiteSpace();
	void setNextPointer();

private:
	void startRead();

public:
	struct ModifierNode
	{
		std::string_view name;
		enum Type
		{
			STRING,
			STRING_NOCASE,
			STRING_CHART,
			STRING_CHART_NOCASE,
			UINT64,
			INT64,
			UINT32,
			INT32,
			UINT16,
			INT16,
			BOOL,
			FLOAT,
			FLOATARRAY
		} type;
	};

	using ModifierOutline = std::vector<std::pair<std::string_view, ModifierNode>>;
protected:
	std::string_view extractModifierName();
	std::optional<ModifierNode> findNode(std::string_view name, const ModifierOutline& list);
	Modifiers::Modifier createModifier(ModifierNode node);
};
