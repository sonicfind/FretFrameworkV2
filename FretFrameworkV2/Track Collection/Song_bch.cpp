#include "Song.h"
#include "File Processing/BCHFileReader.h"
#include "File Processing/BCHFileWriter.h"

void Song::load_bch(const std::filesystem::path& path)
{
	BCHFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	m_tickrate = reader.extract<uint32_t>();
	do
		reader.nextEvent();
	while (reader.isStillCurrentTrack());

	while (reader.isStartOfTrack())
		if (!load_noteTrack(&reader) && !load_tempoMap(&reader) && !load_events(&reader))
			reader.skipUnknownTrack();
}

void Song::save_bch(const std::filesystem::path& path)
{
	BCHFileWriter writer(path);
	save(&writer);
}
