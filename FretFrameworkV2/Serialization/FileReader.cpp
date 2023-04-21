#include "FileReader.h"

const char* FileReader::getStartOfFile() const noexcept
{
	return m_fileData.get();
}

const char* FileReader::getEndOfFile() const noexcept
{
	return m_fileEnd;
}

FileReader::FileReader(const std::filesystem::path& path)
{
	FILE* file;
	if (_wfopen_s(&file, path.c_str(), L"rb") != 0 || !file)
		throw std::runtime_error("Error: " + path.string() + " could not be located");

	_fseek_nolock(file, 0, SEEK_END);
	m_fileSize = _ftell_nolock(file);
	_fseek_nolock(file, 0, SEEK_SET);

	m_fileData = std::make_shared<char[]>(m_fileSize + 1);
	if (_fread_nolock(m_fileData.get(), m_fileSize, 1, file) != 1)
		throw std::runtime_error("Uh, shoot");
	_fclose_nolock(file);
	m_fileData[m_fileSize] = 0;

	m_currentPosition = m_fileData.get();
	m_fileEnd = m_currentPosition + m_fileSize;
}

FileReader::FileReader(const FileReader& file) : m_fileSize(file.m_fileSize), m_currentPosition(file.m_currentPosition), m_fileEnd(file.m_fileEnd) {}

bool FileReader::move(size_t amount)
{
	if (m_currentPosition + amount > m_fileEnd)
		return false;

	m_currentPosition += amount;
	return true;
}
