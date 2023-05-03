#pragma once
#include <filesystem>
#include <fstream>

template <bool byteSwap = false>
class BinaryFileWriter
{
public:
	BinaryFileWriter(const std::filesystem::path& path) : m_file(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary)
	{
		if (!m_file.is_open())
			throw std::runtime_error("Error: " + path.string() + " could not be opened for writing");
	}

	void writeTag(const char(&tag)[5])
	{
		m_file.write(tag, 4);
	}

	template <typename T>
	void write(const T* src, const size_t size)
	{
		if constexpr (byteSwap && (std::is_same<T, uint32_t>::value || std::is_same<T, uint16_t>::value))
		{
			for (size_t index = 0; index < size; index += sizeof(T))
			{
				T buf = *src;
				if constexpr (std::is_same<T, uint32_t>::value)
					buf = _byteswap_ulong(buf);
				else
					buf = _byteswap_ushort(buf);

				m_file.write((char*)&buf, size - index);
			}
		}
		else
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

	auto tell()
	{
		return m_file.tellp();
	}

	void seek(std::streampos pos)
	{
		m_file.seekp(pos);
	}

private:
	std::ofstream m_file;
};
