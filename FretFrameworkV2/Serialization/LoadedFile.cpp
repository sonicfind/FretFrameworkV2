#include "LoadedFile.h"

LoadedFile::LoadedFile(const std::filesystem::path& filepath)
{
	FILE* file;
	if (_wfopen_s(&file, filepath.c_str(), L"rb") != 0 || !file)
		throw std::runtime_error("Error: " + filepath.string() + " could not be located");

	_fseek_nolock(file, 0, SEEK_END);
	m_fileSize = _ftell_nolock(file);
	_fseek_nolock(file, 0, SEEK_SET);

	m_fileData = std::make_shared_for_overwrite<char[]>(m_fileSize + 1);
	if (_fread_nolock(m_fileData.get(), m_fileSize, 1, file) != 1)
		throw std::runtime_error("Uh, shoot");
	_fclose_nolock(file);
	m_fileData[m_fileSize] = 0;
}

LoadedFile::LoadedFile(const std::shared_ptr<char[]>& data, size_t size) : m_fileSize(size), m_fileData(data) {}

const char* LoadedFile::begin() const noexcept
{
	return m_fileData.get();
}

const char* LoadedFile::end() const noexcept
{
	return m_fileData.get() + m_fileSize;
}

MD5 LoadedFile::calculateMD5() const noexcept
{
	return MD5(begin(), m_fileSize);
}
