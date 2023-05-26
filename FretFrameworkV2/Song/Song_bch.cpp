#include "Song.h"
#include "Serialization/BCHFileReader.h"
#include "Serialization/BCHFileWriter.h"

void Song::load_bch(const std::filesystem::path& path)
{
	BCHFileReader reader(path);
	m_tickrate = reader.getTickRate();
	setSustainThreshold();
	traverse(reader);
}
