#pragma once
#include <memory>
#include <filesystem>
#include "MD5.h"

class FileReader
{
public:
	FileReader(const std::filesystem::path& path);
	FileReader(const FileReader& file);
	FileReader(FileReader&&) = default;

	virtual bool move(size_t amount);
	MD5 calculateMD5() const noexcept;

protected:
	[[nodiscard]] const char* getStartOfFile() const noexcept;
	[[nodiscard]] const char* getEndOfFile() const noexcept;

private:
	size_t m_fileSize = 0;
	std::shared_ptr<char[]> m_fileData;
	const char* m_fileEnd = nullptr;

protected:
	const char* m_currentPosition = nullptr;
	const char* m_next = nullptr;
};
