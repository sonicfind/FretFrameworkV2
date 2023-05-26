#include "LibraryEntry.h"
#include "Serialization/BCHFileReader.h"

void LibraryEntry::scan_bch(const LoadedFile& file)
{
	BCHFileReader reader(file);
	traverse(reader);
}
