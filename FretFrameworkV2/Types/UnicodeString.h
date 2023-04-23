#pragma once
#include <string>
#include <stdexcept>
#include <fstream>

class UnicodeString
{
	static constexpr char s_LOWER[] =
	{
		0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
		16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
		32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
		48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
		64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91,  92,  93,  94,  95,
		96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	};

	static constexpr char s_UPPER[] =
	{
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,  12,  13,  14,  15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  28,  29,  30,  31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,  44,  45,  46,  47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,  60,  61,  62,  63,
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,  76,  77,  78,  79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,  92,  93,  94,  95,
		96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,  76,  77,  78,  79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127,
	};

	class InvalidCharacterException : public std::runtime_error
	{
	public:
		InvalidCharacterException(char32_t value);
	};

	std::u32string m_string;
	std::u32string m_string_lowercase;
	std::u32string m_string_uppercase;

public:
	UnicodeString() = default;
	UnicodeString(const UnicodeString&) = default;
	UnicodeString(UnicodeString&&) = default;
	UnicodeString& operator=(const UnicodeString&) = default;
	UnicodeString& operator=(UnicodeString&&) = default;

	UnicodeString(std::u32string&& str);
	UnicodeString(const std::u32string& str);
	UnicodeString& operator=(const std::u32string& str);
	UnicodeString& operator=(std::u32string&& str);

	UnicodeString(std::string_view str);
	UnicodeString& operator=(std::string_view str);

	constexpr void setCasedStrings() noexcept;
	void writeToWebTypedFile(std::fstream& outFile) const;

	static char s_writeBuffer[5];
	static char* s_bufferStart;
	static size_t s_bufferSize;
	std::string toString() const;

	std::u32string& get() { return m_string; }
	const std::u32string& get() const { return m_string; }

	std::u32string& getLowerCase() { return m_string_lowercase; }
	const std::u32string& getLowerCase() const { return m_string_lowercase; }

	std::u32string& getUpperCase() { return m_string_uppercase; }
	const std::u32string& getUpperCase() const { return m_string_uppercase; }

	// Comapres by the lowercase versions of the string
	bool operator==(const UnicodeString& str) const;

	// Comapres by the lowercase versions of the string
	bool operator<(const UnicodeString& str) const;

	int compare(const UnicodeString& str) const;
	
	operator std::string() const { return toString(); }
	friend std::ostream& operator<<(std::ostream& outFile, const UnicodeString& str);
	constexpr std::u32string* operator->() const { return (std::u32string*)&m_string; }

	char32_t& operator[](size_t i) { return m_string[i]; }
	const char32_t& operator[](size_t i) const { return m_string[i]; }


	static std::u32string strToU32(const std::string_view str);
	static std::u32string bufferToU32(const unsigned char* dataPtr, size_t length);
	static std::string U32ToStr(const std::u32string& u32);
	static std::u32string U32FromWebTypedFile(const char*& dataPtr);
	static void U32ToWebTypedFile(const std::u32string& u32, std::fstream& outFile);
};

