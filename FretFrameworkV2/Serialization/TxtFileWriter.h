#pragma once
#include <filesystem>
#include <fstream>
class TxtFileWriter
{
public:
	TxtFileWriter(const std::filesystem::path& path) : m_file(path, std::ios_base::out | std::ios_base::trunc)
	{
		if (!m_file.is_open())
			throw std::runtime_error("Error: " + path.string() + " could not be opened for writing");

		m_file.setf(std::ios_base::boolalpha);
	}

	template <typename T>
	void write(T value) noexcept
	{
		m_file << ' ' << value;
	}

	~TxtFileWriter() { m_file.close(); }

protected:
	std::ofstream m_file;
};
