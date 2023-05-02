#include "Song.h"
#include "Serialization/BCHFileReader.h"
#include "Serialization/BCHFileWriter.h"

void Song::load_bch(const std::filesystem::path& path)
{
	BCHFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	m_tickrate = reader.extract<uint32_t>();
	reader.nextEvent();
	traverse(reader);
}

void Song::save_bch(const std::filesystem::path& path)
{
	BCHFileWriter writer(path);
	save(writer);
}
