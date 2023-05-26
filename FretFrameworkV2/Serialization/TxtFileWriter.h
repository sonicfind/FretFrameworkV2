#pragma once
#include <filesystem>
#include <fstream>
class TxtFileWriter
{
public:
	TxtFileWriter(const std::filesystem::path& path);

	template <typename T>
	void write(T value) noexcept
	{
		m_file << ' ' << value;
	}

	virtual ~TxtFileWriter() {}

protected:
	std::ofstream m_file;
};
