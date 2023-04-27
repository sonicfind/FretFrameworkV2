#include "Song.h"
#include "Serialization/ChtFileWriter.h"

void Song::load_cht(const std::filesystem::path& path)
{
	ChtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	if (load_songInfo_cht(reader) > 1)
		load(reader);
	else
		load_cht_V1(reader);
}

void Song::save_cht(const std::filesystem::path& path)
{
	ChtFileWriter writer(path);
	save(writer);
}

int Song::load_songInfo_cht(ChtFileReader& reader)
{
	int version = 0;
	while (reader.isStillCurrentTrack())
	{
		const std::string_view modifier = reader.parseModifierName();
		if (modifier == "Resolution")
			m_tickrate = reader.extract<uint32_t>();
		else if (modifier == "FileVersion")
			version = reader.extract<uint32_t>();
		reader.nextEvent();
	}
	return version;
}
