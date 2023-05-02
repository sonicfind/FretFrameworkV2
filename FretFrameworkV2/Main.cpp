#include "Song/Song.h"
#include "Song Library/SongLibrary.h"
#include <iostream>

std::string parseInput();
std::vector<std::u32string> getDirectories();
void loadSong();
void startClock();
void stopClock(std::string_view str);

std::chrono::steady_clock::time_point g_t1;
std::chrono::steady_clock::time_point g_t2;
long long g_count;

int main()
{
	SongLibrary library;
	
	startClock();
	if (library.runPartialScan())
	{
		stopClock("Partial scan");
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
				startClock();
				library.runFullScan(directories);
				stopClock("Full scan");
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

void startClock()
{
	g_t1 = std::chrono::high_resolution_clock::now();
}

void stopClock(std::string_view str)
{
	g_t2 = std::chrono::high_resolution_clock::now();
	g_count = std::chrono::duration_cast<std::chrono::microseconds>(g_t2 - g_t1).count();
	std::cout << str << " took " << g_count / 1000.0 << " milliseconds\n";
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