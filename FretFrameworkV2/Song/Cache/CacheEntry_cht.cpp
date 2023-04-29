#include "CacheEntry.h"

using ModifierNode = TxtFileReader::ModifierNode;
const TxtFileReader::ModifierOutline MODIFIER_LIST =
{
	{ "Album",        { "album", ModifierNode::STRING_CHART } },
	{ "Artist",       { "artist", ModifierNode::STRING_CHART } },
	{ "Charter",      { "charter", ModifierNode::STRING_CHART } },
	{ "Difficulty",   { "diff_band", ModifierNode::INT32 } },
	{ "FileVersion",  { "FileVersion", ModifierNode::UINT16 } },
	{ "Genre",        { "genre", ModifierNode::STRING_CHART } },
	{ "Name",         { "name", ModifierNode::STRING_CHART } },
	{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT } },
	{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT } },
	{ "Year",         { "year", ModifierNode::STRING_CHART } },
};

bool CacheEntry::scan_cht(const std::filesystem::path& path)
{
	static constexpr auto isModifierDefault = [](const Modifiers::Modifier & modifier)
	{
		if (modifier.getName() == "name")             return modifier.getValue<UnicodeString>() == s_DEFAULT_NAME;
		else if (modifier.getName() == "artist")      return modifier.getValue<UnicodeString>() == s_DEFAULT_ARTIST;
		else if (modifier.getName() == "album")       return modifier.getValue<UnicodeString>() == s_DEFAULT_ALBUM;
		else if (modifier.getName() == "genre")       return modifier.getValue<UnicodeString>() == s_DEFAULT_GENRE;
		else if (modifier.getName() == "year")        return modifier.getValue<UnicodeString>() == s_DEFAULT_YEAR;
		else if (modifier.getName() == "charter")     return modifier.getValue<UnicodeString>() == s_DEFAULT_CHARTER;
		else if (modifier.getName() == "diff_band")   return modifier.getValue<int32_t>() == -1;
		return false;
	};

	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	auto modifiers = reader.extractModifiers(MODIFIER_LIST);
	bool updateIni = false;
	int version = 0;
	for (auto& mod : modifiers)
	{
		if (mod.getName() == "FileVersion")
			version = mod.getValue<uint32_t>();
		else if (auto iter = getModifier(mod.getName()); iter != m_modifiers.end())
		{
			if (isModifierDefault(*iter))
			{
				*iter = std::move(mod);
				updateIni = true;
			}
		}
		else
		{
			m_modifiers.push_back(std::move(mod));
			updateIni = true;
		}
	}

	if (version > 1)
		scan(reader);
	else
		scan_cht_V1(reader);
	return updateIni;
}
