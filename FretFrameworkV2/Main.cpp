#include "Song/Song.h"
#include "Song Library/SongLibrary.h"
#include "TaskQueue/TaskQueue.h"
#include <iostream>

std::string parseInput();
std::vector<std::u32string> getDirectories();
void loadSong();
void setThreads();
void startClock();
void stopClock(std::string_view str);

std::chrono::steady_clock::time_point g_t1;
std::chrono::steady_clock::time_point g_t2;
long long g_count;

int main()
{
	TaskQueue::getInstance();
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
		std::cout << "T - Set Max Threads\n";
		std::cout << "(Leave empty to go to exit)\n";
		std::cout << "Input: ";
		std::string input = parseInput();
		if (input.empty())
			break;
		
		if (input.size() > 1)
		{
			std::cout << "Invalid input\n";
			continue;
		}

		
		switch (std::toupper(input.front()))
		{
		case 'S':
		{
			std::cout << '\n';
			const std::vector<std::u32string> directories = getDirectories();
			library.clear();
			startClock();
			library.runFullScan(directories);
			stopClock("Full scan");
			std::cout << "# of Songs: " << library.getNumSongs() << '\n';
		}
			break;
		case 'L':
			std::cout << '\n';
			loadSong();
			break;
		case 'T':
			std::cout << '\n';
			setThreads();
			break;
		default:
			std::cout << "Invalid option\n";
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
	static const std::pair<std::string_view, ChartType> CHARTTYPES[] =
	{
		{ ".bch",	ChartType::BCH },
		{ ".cht",   ChartType::CHT },
		{ ".mid",   ChartType::MID },
		{ ".midi",  ChartType::MID },
		{ ".chart", ChartType::CHT },
	};

	while (true)
	{
		std::cout << "Drag and drop a chart File: ";
		std::string file = parseInput();
		if (file.empty())
			break;

		for (size_t i = 0; i < std::size(CHARTTYPES); ++i)
		{
			if (file.ends_with(CHARTTYPES[i].first))
			{
				const std::filesystem::path path = UnicodeString::strToU32(file);
				startClock();
				Song song(path.parent_path());
				song.loadIni();
				if (!song.load(path, CHARTTYPES[i].second, true))
					break;
				stopClock("Song load");

				while (true)
				{
					std::cout << "B - Save BCH\n";
					std::cout << "C - Save CHT\n";
					std::cout << "M - Save MID\n";
					std::cout << "(Leave empty to not save)\n";
					std::cout << "Input: ";
					std::string input = parseInput();
					if (input.empty())
						break;

					if (input.size() > 1)
						std::cout << "Invalid input\n";
					else if (input.front() == 'b' || input.front() == 'B')
					{
						if (song.save(ChartType::BCH))
							std::cout << "Successfully saved .bch\n";
						else
							std::cout << "Saved failed\n";
					}
					else if (input.front() == 'c' || input.front() == 'C')
					{
						if (song.save(ChartType::CHT))
							std::cout << "Successfully saved .cht\n";
						else
							std::cout << "Saved failed\n";
					}
					else if (input.front() == 'm' || input.front() == 'M')
					{
						if (song.save(ChartType::MID))
							std::cout << "Successfully saved .mid\n";
						else
							std::cout << "Saved failed\n";
					}
					else
						std::cout << "Invalid option\n";
					std::cout << '\n';
				}
				song.clear();
				break;
			}
		}
		std::cout << '\n';
	}
}

void setThreads()
{
	TaskQueue& queue = TaskQueue::getInstance();
	while (true)
	{
		std::cout << "Current # of threads: " << queue.getNumThreads() << "\n";
		std::cout << "Provide number of threads (Max: " << std::thread::hardware_concurrency() << "): ";
		std::string input = parseInput();
		if (input.empty())
			break;

		char* end;
		unsigned long num = std::strtoul(input.data(), &end, 0);
		if (end == input.data())
		{
			std::cout << "Invalid input\n\n";
			continue;
		}
		else if (num > std::thread::hardware_concurrency())
		{
			std::cout << "Max exceeded\n\n";
			continue;
		}

		queue.startThreads(num);
		std::cout << '\n';
	}
}
