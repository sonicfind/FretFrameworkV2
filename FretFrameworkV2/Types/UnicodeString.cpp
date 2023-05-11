#include "UnicodeString.h"
#include "utf_utils.h"

UnicodeString::InvalidCharacterException::InvalidCharacterException(char32_t value)
	: std::runtime_error("Character value in a u32string cannot exceed 1114111 (value: " + std::to_string(value) + ")") {}

UnicodeString::UnicodeString(std::u32string&& str) : m_string(std::move(str)) { setCasedStrings(); }
UnicodeString::UnicodeString(const std::u32string& str) : m_string(str) { setCasedStrings(); }

UnicodeString& UnicodeString::operator=(const std::u32string& str)
{
	m_string = str;
	setCasedStrings();
	return *this;
}

UnicodeString& UnicodeString::operator=(std::u32string&& str)
{
	m_string = std::move(str);
	setCasedStrings();
	return *this;
}

UnicodeString::UnicodeString(std::string_view str) : m_string(strToU32(str)) { setCasedStrings(); }

UnicodeString& UnicodeString::operator=(std::string_view str)
{
	m_string = strToU32(str);
	setCasedStrings();
	return *this;
}

constexpr void UnicodeString::setCasedStrings() noexcept
{
	const size_t size = m_string.size();
	const char32_t* const arr = m_string.c_str();
	m_string_lowercase.resize_and_overwrite(size,
		[size, arr](char32_t* buf, std::size_t)
		{
			for (size_t i = 0; i < size; ++i)
			buf[i] = std::tolower(arr[i]);
			return size;
		});

	m_string_uppercase.resize_and_overwrite(size,
		[size, arr](char32_t* buf, std::size_t)
		{
			for (size_t i = 0; i < size; ++i)
			buf[i] = std::toupper(arr[i]);
			return size;
		});
}

std::string UnicodeString::toString() const
{
	return U32ToStr(m_string);
}

bool UnicodeString::operator==(const UnicodeString& str) const
{
	return m_string_lowercase == str.m_string_lowercase;
}

int UnicodeString::compare(const UnicodeString& str) const
{
	return m_string_lowercase.compare(str.m_string_lowercase);
}

std::u32string UnicodeString::bufferToU32(const unsigned char* dataPtr, size_t length)
{
	std::u32string str;
	str.resize_and_overwrite(length,
		[dataPtr, endPtr = dataPtr + length](char32_t* buf, std::size_t)
		{
			return uu::UtfUtils::SseBigTableConvert(dataPtr, endPtr, buf);
		});
	return str;
}

std::u32string UnicodeString::strToU32(const std::string_view str)
{
	return bufferToU32((const unsigned char*)str.data(), str.size());
}

std::string UnicodeString::U32ToStr(const std::u32string& u32)
{
	std::string str;
	str.resize_and_overwrite(u32.size() * 4,
		[u32Data = u32.data(), u32Size = u32.size()](char* buf, std::size_t)
		{
			size_t size = 0;
			for (size_t i = 0; i < u32Size; ++i)
			{
				uint64_t movement = uu::UtfUtils::GetCodeUnits(u32Data[i], (unsigned char*)buf + size);
				size += movement;
				if (movement == 0)
					buf[size++] = '_';
			}

			return size;
		});
	return str;
}
