#pragma once
#include "FileReader.h"

template <bool byteSwap = false>
class BinaryFileReader : public FileReader
{
public:
	using FileReader::FileReader;
	[[nodiscard]] bool checkTag(const char(&tag)[5])
	{
		if (strncmp(m_currentPosition, tag, 4) != 0)
			return false;
		return FileReader::move(4);
	}

	template <typename T, bool useVirtual = true>
	bool extract(T* dst, const size_t size)
	{
		const auto pos = m_currentPosition;
		if constexpr (useVirtual)
		{
			if (!move(size))
				return false;
		}
		else if (!FileReader::move(size))
			return false;


		memcpy(dst, pos, size);
		if constexpr (byteSwap && (std::is_same<T, uint32_t>::value || std::is_same<T, uint16_t>::value))
		{
			const size_t count = size / sizeof(T);
			for (size_t index = 0; index < count; index++)
			{
				if constexpr (std::is_same<T, uint32_t>::value)
					dst[index] = _byteswap_ulong(dst[index]);
				else
					dst[index] = _byteswap_ushort(dst[index]);
			}
		}
		return true;
	}

	template <typename T, bool useVirtual = true>
	bool extract(T& value, const size_t size)
	{
		if (size > sizeof(T))
			throw std::runtime_error("Size value exceeds size of type");
		return extract<T, useVirtual>(&value, size);
	}

	template <typename T, bool useVirtual = true>
	bool extract(T& value)
	{
		if constexpr (sizeof(T) == 1)
		{
			value = *reinterpret_cast<const T*>(m_currentPosition);
			if constexpr (useVirtual)
				return move(1);
			else
				return FileReader::move(1);
		}
		return extract<T, useVirtual>(value, sizeof(T));
	}

	template <typename T, bool useVirtual = true>
	[[nodiscard]] T extract()
	{
		T value;
		if (!extract<T, useVirtual>(value))
			throw std::runtime_error("Could not parse data");
		return value;
	}

protected:
	const char* m_next = nullptr;

	template <typename T>
	[[nodiscard]] T extract_nonvirtual()
	{
		T value;
		if (!extract<T, false>(value))
			throw std::runtime_error("Could not parse data");
		return value;
	}
};
