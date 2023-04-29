#pragma once
#include "Serialization/IniFileReader.h"
#include "SimpleFlatMap/SimpleFlatMap.h"
namespace Ini
{
	using ModifierNode = IniFileReader::ModifierNode;

	template <size_t SIZE>
	SimpleFlatMap<std::vector<Modifiers::Modifier>, std::string_view> ReadIniFile(const std::filesystem::path& path, const std::pair<std::string_view, IniFileReader::ModifierOutline> (&sections)[SIZE])
	{
		IniFileReader reader(path);
		SimpleFlatMap<std::vector<Modifiers::Modifier>, std::string_view> modifierMap;

		auto const readSection = [&]
		{
			for (const std::pair<std::string_view, IniFileReader::ModifierOutline>& section : sections)
			{
				if (reader.validateSection(section.first))
				{
					modifierMap[section.first] = reader.extractModifiers(section.second);
					return true;
				}
			}
			return false;
		};

		while (reader.isStartOfSection())
			if (!readSection())
				reader.skipSection();
		return modifierMap;
	}
	std::vector<Modifiers::Modifier> ReadSongIniFile(const std::filesystem::path& path);
};
