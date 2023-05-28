#pragma once
#include "FileReader.h"

class BinaryFileReader : public FileReader
{
public:
	BinaryFileReader(const std::filesystem::path& path);
	BinaryFileReader(const LoadedFile& file);

	void exitSection();

	[[nodiscard]] bool checkTag(const char(&tag)[5]);
	bool move(size_t amount);

	template <typename T>
	bool extract(T* dst, const size_t size)
	{
		const auto pos = m_currentPosition;
		if (!move(size))
			return false;

		memcpy(dst, pos, size);
		return true;
	}

	template <typename T>
	bool extract(T& value, const size_t size)
	{
		if (size > sizeof(T))
			throw std::runtime_error("Size value exceeds size of type");
		return extract<T>(&value, size);
	}

	template <typename T>
	bool extract(T& value)
	{
		return extract<T>(value, sizeof(T));
	}

	template <typename T>
	[[nodiscard]] T extract()
	{
		T value;
		if (!extract<T>(value))
			throw std::runtime_error("Could not parse data");
		return value;
	}

protected:
	void enterSection(uint64_t length);

protected:
	std::vector<const char*> m_ends;
};
