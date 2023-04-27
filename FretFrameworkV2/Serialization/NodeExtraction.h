#pragma once
#include <string>
#include <optional>

namespace NodeExtraction
{
	template <class T, size_t SIZE>
	static std::optional<T> FindNode(const std::pair<std::string_view, T>(&_MODIFIERLIST)[SIZE], const std::string_view _modifierName)
	{
		const auto pairIter = std::lower_bound(std::begin(_MODIFIERLIST), std::end(_MODIFIERLIST), _modifierName,
			[](const std::pair<std::string_view, T>& pair, const std::string_view str)
			{
				return pair.first < str;
			});

		if (pairIter == std::end(_MODIFIERLIST) || _modifierName != pairIter->first)
			return {};

		return pairIter->second;
	}
};
