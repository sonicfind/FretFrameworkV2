#pragma once
#include "LoadedFile.h"

class FileReader
{
public:
	FileReader(const std::filesystem::path& path);
	FileReader(const LoadedFile& file);

protected:
	const LoadedFile m_file;
	const char* m_currentPosition = nullptr;
};
