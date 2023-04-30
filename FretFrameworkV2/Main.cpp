#include "Song/Song.h"
#include "Song Library/Entry/LibraryEntry.h"
#include <iostream>
int main()
{
	Song song;

	std::string filename;
	std::cout << "Drag and drop a file:";
	std::getline(std::cin, filename);

	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	auto t1 = std::chrono::high_resolution_clock::now();
	if (song.load(filename))
	{
		auto t2 = std::chrono::high_resolution_clock::now();
		long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "Load took " << count / 1000.0 << " milliseconds\n";

		//if (filename.ends_with(".chart") || filename.ends_with(".mid"))
			//song.save(filename);
	}
	else
		std::cout << "Load failed\n";
	
	
	std::filesystem::directory_entry ini(std::filesystem::path(filename).replace_filename(U"song.ini"));
	std::filesystem::directory_entry chart(filename);

	LibraryEntry entry(chart.last_write_time());
	t1 = std::chrono::high_resolution_clock::now();
	if (ini.exists())
		entry.readIni(ini.path(), ini.last_write_time());
	if (entry.scan(filename))
	{
		auto t2 = std::chrono::high_resolution_clock::now();
		long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "Scan took " << count / 1000.0 << " milliseconds\n";
	}
	else
		std::cout << "Scan failed\n"; 

	std::cout << "Press Enter to Exit" << std::endl;
	std::getline(std::cin, filename);
	return 0;
}