#pragma once
#include <filesystem>
#include <fstream>

class BinaryFileWriter
{
public:
	BinaryFileWriter(const std::filesystem::path& path);
	virtual ~BinaryFileWriter() {}

	template <typename T>
	void write(const T* src, const size_t size)
	{
		m_file.write((char*)src, size);
	}

	template <typename T>
	void write(const T& value, const size_t size)
	{
		if (size > sizeof(T))
			throw std::runtime_error("Size value exceeds size of type");
		write(&value, size);
	}

	template <typename T>
	void write(const T& value)
	{
		write(value, sizeof(T));
	}

	template <typename T>
	friend BinaryFileWriter& operator<<(BinaryFileWriter& file, const T& value)
	{
		file.write(value);
		return file;
	}

	std::streampos tell();
	void seek(std::streampos pos);

private:
	std::ofstream m_file;
};
