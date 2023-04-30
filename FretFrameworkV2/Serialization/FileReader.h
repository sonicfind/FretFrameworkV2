#pragma once
#include "LoadedFile.h"

class FileReader
{
public:
	FileReader(const std::filesystem::path& path);
	FileReader(const LoadedFile& file);

	virtual bool move(size_t amount);

protected:
	const LoadedFile m_file;
	const char* m_currentPosition = nullptr;
	const char* m_next = nullptr;
};
