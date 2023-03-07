#include "Track Collection/TrackCollection.h"
#include <iostream>
int main()
{
	Collection collection;

	std::string filename;
	std::cout << "Drag and drop a file:";
	std::getline(std::cin, filename);

	if (filename[0] == '\"')
		filename = filename.substr(1, filename.length() - 2);

	auto t1 = std::chrono::high_resolution_clock::now();
	collection.load(filename);
	auto t2 = std::chrono::high_resolution_clock::now();
	long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Load took " << count / 1000.0 << " milliseconds\n";
	std::cout << "yay\n" << std::endl;

	t1 = std::chrono::high_resolution_clock::now();
	collection.load(filename);
	t2 = std::chrono::high_resolution_clock::now();
	count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Load took " << count / 1000.0 << " milliseconds\n";
	std::cout << "yay 2" << std::endl;

	if (filename.ends_with(".chart") || filename.ends_with(".mid"))
		collection.save(filename);

	std::cout << "Press Enter to Exit" << std::endl;
	std::getline(std::cin, filename);

	return 0;
}