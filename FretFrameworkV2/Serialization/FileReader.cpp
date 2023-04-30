#include "FileReader.h"

FileReader::FileReader(const std::filesystem::path& path) : m_file(path), m_currentPosition(m_file.begin()) {}
FileReader::FileReader(const LoadedFile& file) : m_file(file), m_currentPosition(m_file.begin()) {}

bool FileReader::move(size_t amount)
{
	if (m_currentPosition + amount > m_file.end())
		return false;

	m_currentPosition += amount;
	return true;
}
