#include "FileReader.h"

FileReader::FileReader(const std::filesystem::path& path) : m_file(path), m_currentPosition(m_file.begin()) {}
FileReader::FileReader(const LoadedFile& file) : m_file(file), m_currentPosition(m_file.begin()) {}
