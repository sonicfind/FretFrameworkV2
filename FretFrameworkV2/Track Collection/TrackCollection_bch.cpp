#include "TrackCollection.h"
#include "File Processing/BCHFileReader.h"
#include "File Processing/BCHFileWriter.h"

void Collection::load_bch(const std::filesystem::path& path)
{
	BCHFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	m_tickrate = reader.extract<uint32_t>();
	reader.skipTrack();
	reader.endTrack();

	while (reader.isStartOfTrack())
	{
		if (!load_instrumentTrack(&reader) &&
			!load_vocalTrack(&reader) &&
			!load_tempoMap(&reader) &&
			!load_events(&reader))
		{
			reader.processUnknownTrack();
			reader.skipTrack();
			reader.endTrack();
		}
	}
}

void Collection::save_bch(const std::filesystem::path& path)
{
	BCHFileWriter writer(path);
	save(&writer);
}
