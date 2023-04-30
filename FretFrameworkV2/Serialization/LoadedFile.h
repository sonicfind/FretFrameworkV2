#pragma once
#include <memory>
#include <filesystem>
#include "Types/MD5.h"

class LoadedFile
{
public:
	LoadedFile(const std::filesystem::path& filepath);
	LoadedFile(const char* data, size_t size);

	[[nodiscard]] const char* begin() const noexcept;
	[[nodiscard]] const char* end() const noexcept;
	MD5 calculateMD5() const noexcept;

private:
	size_t m_fileSize = 0;
	std::shared_ptr<char[]> m_fileData;
};
