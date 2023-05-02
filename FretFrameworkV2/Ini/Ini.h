#pragma once
#include "Serialization/IniFileReader.h"
#include "Serialization/IniFileWriter.h"
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

	template <size_t SIZE>
	void WriteIniFile(const std::filesystem::path& path, const std::pair<std::string_view, std::vector<Modifiers::Modifier>>(&sections)[SIZE])
	{
		IniFileWriter writer(path);
		for (const auto& section : sections)
		{
			writer.writeSection(section.first);
			writer.writeModifiers(section.second);
		}
	}

	std::vector<Modifiers::Modifier> ReadSongIniFile(const std::filesystem::path& path);
	std::vector<Modifiers::Modifier> GrabLoadingPhrase(const std::filesystem::path& path);
	void WriteSongIniFile(const std::filesystem::path& path, const std::vector<Modifiers::Modifier>& modifiers);
};
