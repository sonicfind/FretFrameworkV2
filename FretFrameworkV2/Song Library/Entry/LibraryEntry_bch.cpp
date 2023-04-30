#include "LibraryEntry.h"
#include "Serialization/BCHFileReader.h"

void LibraryEntry::scan_bch(const LoadedFile& file)
{
	BCHFileReader reader(file);
	if (!reader.validateHeaderTrack())
		throw std::runtime_error("Header track not found at the start of the file");

	reader.nextEvent();
	scan(reader);
}
