#include "Song/Song.h"
#include "Song Library/SongLibrary.h"
#include <iostream>
int main()
{
	SongLibrary library;
	std::string filename;
	std::cout << "Drag and drop a Directory:";
	std::getline(std::cin, filename);

	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	std::vector<std::filesystem::path> directories = { filename };
	auto t1 = std::chrono::high_resolution_clock::now();
	library.clear();
	library.readFromCacheFile();
	library.scan(directories);
	library.finalize();
	library.writeToCacheFile();
	auto t2 = std::chrono::high_resolution_clock::now();
	long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Full scan took " << count / 1000.0 << " milliseconds\n";
	std::cout << "# of Songs: " << library.getNumSongs() << '\n';

	std::cout << "Press Enter to Exit" << std::endl;
	std::getline(std::cin, filename);
	return 0;
}