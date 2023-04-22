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
		if constexpr (byteSwap)
		{
			if constexpr (std::is_same<T, uint32_t>::value)
				*dst = _byteswap_ulong(*dst);
			else if constexpr (std::is_same<T, uint16_t>::value)
				*dst = _byteswap_ushort(*dst);
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

	template <typename T>
	[[nodiscard]] T extract()
	{
		T value;
		if (!extract<T, true>(value))
			throw std::runtime_error("Could not parse data");
		return value;
	}

protected:
	template <typename T>
	[[nodiscard]] T extract_nonvirtual()
	{
		T value;
		if (!extract<T, false>(value))
			throw std::runtime_error("Could not parse data");
		return value;
	}
};
