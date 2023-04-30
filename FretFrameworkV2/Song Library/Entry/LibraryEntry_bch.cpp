#include "LibraryEntry.h"
#include "Serialization/BCHFileReader.h"
#include "Serialization/BCHFileWriter.h"

void LibraryEntry::scan_bch(const std::filesystem::path& path)
{
	BCHFileReader reader(path);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	reader.nextEvent();
	scan(reader);
}
