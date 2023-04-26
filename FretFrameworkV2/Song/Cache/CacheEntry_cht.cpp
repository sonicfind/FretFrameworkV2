#include "CacheEntry.h"
#include "Serialization/ChtFileWriter.h"

void CacheEntry::scan_cht(const std::filesystem::path& path)
{
	TxtFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("[Song] track expected at the start of the file");

	if (load_songInfo_cht(reader) > 1)
		scan(&reader);
	else
		scan_cht_V1(reader);
}

int CacheEntry::load_songInfo_cht(TxtFileReader& reader)
{
	int version = 0;
	while (reader.isStillCurrentTrack())
	{
		const std::string_view modifier = reader.parseModifierName();
		if (modifier == "FileVersion")
			version = reader.extract<uint32_t>();
		reader.nextEvent();
	}
	return version;
}
