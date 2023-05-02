#include "Song/Song.h"
#include "Song Library/SongLibrary.h"
#include <iostream>

std::string parseInput();
std::vector<std::u32string> getDirectories();
void loadSong();

int main()
{
	SongLibrary library;
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	long long count;
	if (library.runPartialScan())
	{
		t2 = std::chrono::high_resolution_clock::now();
		count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << "Partial scan took " << count / 1000.0 << " milliseconds\n";
		std::cout << "# of Songs: " << library.getNumSongs() << '\n';
	}

	while (true)
	{
		std::cout << "S - Full Scan\n";
		std::cout << "L - Load Song\n";
		std::cout << "Input: ";
		std::string input = parseInput();
		if (input.empty())
			break;
		
		if (input.size() > 1)
			std::cout << "Invalid input\n";
		else
		{
			switch (std::toupper(input.front()))
			{
			case 'S':
			{
				std::cout << '\n';
				const std::vector<std::u32string> directories = getDirectories();
				t1 = std::chrono::high_resolution_clock::now();
				library.runFullScan(directories);
				t2 = std::chrono::high_resolution_clock::now();
				count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
				std::cout << "Full scan took " << count / 1000.0 << " milliseconds\n";
				std::cout << "# of Songs: " << library.getNumSongs() << '\n';
			}
				break;
			case 'L':
				break;
			default:
				std::cout << "Invalid option\n";
			}
		}

		std::cout << '\n';
	}

	std::cout << "Press Enter to Exit" << std::endl;
	std::string buf;
	std::getline(std::cin, buf);
	return 0;
}

std::string parseInput()
{
	std::string input;
	std::getline(std::cin, input);

	if (input.size() >= 2 && input.front() == '\"' && input.back() == '\"')
		input = input.substr(1, input.length() - 2);
	return input;
}

std::vector<std::u32string> getDirectories()
{
	std::vector<std::u32string> directories;
	while (true)
	{
		std::cout << "Drag and drop a Directory (Leave empty to start scan):";
		std::string directory = parseInput();
		if (directory.empty())
			break;

		directories.push_back(UnicodeString::strToU32(directory));
		std::cout << '\n';
	}
	return directories;
}

void loadSong()
{
	Song song;
	while (true)
	{
		std::cout << "Drag and drop a chart file or directrory: ";
		std::string file = parseInput();
		if (file.empty())
			break;

		std::filesystem::directory_entry entry(UnicodeString::strToU32(file));
		//std::filesystem::directory_entry entry(UnicodeString::strToU32(file));
		std::filesystem::directory_iterator iter;
		
	}
}