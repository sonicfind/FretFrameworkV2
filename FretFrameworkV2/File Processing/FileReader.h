#pragma once
#include <memory>
#include <filesystem>

class FileReader
{
public:
	FileReader(const std::filesystem::path& path);
	FileReader(const FileReader& file);
	FileReader(FileReader&&) = default;

	virtual bool move(size_t amount);

protected:
	[[nodiscard]] const char* getStartOfFile() const noexcept;
	[[nodiscard]] const char* getEndOfFile() const noexcept;

private:
	size_t m_fileSize = 0;
	std::shared_ptr<char[]> m_fileData;
	const char* m_fileEnd = nullptr;

protected:
	const char* m_currentPosition = nullptr;
};
