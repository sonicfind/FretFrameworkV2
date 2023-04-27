#pragma once
#include <fstream>
#include <assert.h>
#include "Types/UnicodeString.h"
#include <variant>

namespace Modifiers
{
	enum class Type
	{
		STRING,
		STRING_NOCASE,
		UINT32,
		INT32,
		UINT16,
		BOOL,
		FLOAT,
		FLOATARRAY,
		NONE
	};

	class Modifier
	{
		std::string_view m_name;
		Type m_type = Type::NONE;
		alignas(UnicodeString) char m_BUFFER[sizeof(UnicodeString)];
	public:
		constexpr Modifier(std::string_view name) : m_name(name) {}
		constexpr Modifier(const Modifier& other) noexcept : m_name(other.m_name), m_type(other.m_type)
		{
			if (m_type == Type::STRING)
				new(m_BUFFER) UnicodeString(other.cast<UnicodeString>());
			else if (m_type == Type::STRING_NOCASE)
				new(m_BUFFER) std::u32string(other.cast<std::u32string>());
			else
				memcpy(m_BUFFER, other.m_BUFFER, sizeof(m_BUFFER));
		}

		Modifier(Modifier&& other) noexcept : m_name(other.m_name), m_type(other.m_type)
		{
			memcpy(m_BUFFER, other.m_BUFFER, sizeof(m_BUFFER));
			other.m_type = Type::NONE;
		}

		constexpr void set(std::u32string&& string)
		{
			reset();
			m_type = Type::STRING_NOCASE;
			new(m_BUFFER) std::u32string(std::move(string));
		}

		constexpr void set(UnicodeString&& string)
		{
			reset();
			m_type = Type::STRING;
			new(m_BUFFER) UnicodeString(std::move(string));
		}

		constexpr void set(const UnicodeString& string)
		{
			reset();
			m_type = Type::STRING;
			new(m_BUFFER) UnicodeString(string);
		}

		constexpr void set(uint32_t u32)
		{
			reset();
			m_type = Type::UINT32;
			new(m_BUFFER) uint32_t(u32);
		}

		constexpr void set(int32_t i32)
		{
			reset();
			m_type = Type::INT32;
			new(m_BUFFER) int32_t(i32);
		}

		constexpr void set(uint16_t u16)
		{
			reset();
			m_type = Type::UINT16;
			new(m_BUFFER) uint16_t(u16);
		}

		constexpr void set(bool bl)
		{
			reset();
			m_type = Type::BOOL;
			new(m_BUFFER) bool(bl);
		}

		constexpr void set(float flt)
		{
			reset();
			m_type = Type::FLOAT;
			new(m_BUFFER) float(flt);
		}

		constexpr void set(float flt1, float flt2)
		{
			reset();
			m_type = Type::FLOATARRAY;
			new(m_BUFFER) float[2](flt1, flt2);
		}

		constexpr void reset()
		{
			if (m_type == Type::STRING)
				destruct<UnicodeString>();
			else if (m_type == Type::STRING_NOCASE)
				destruct<std::u32string>();
			m_type = Type::NONE;
		}

	private:
		template <class T>
		T& cast()
		{
			return *reinterpret_cast<T*>(m_BUFFER);
		}

		template <class T>
		const T& cast() const
		{
			return *reinterpret_cast<const T*>(m_BUFFER);
		}

		template <class T>
		void destruct()
		{
			reinterpret_cast<T*>(m_BUFFER)->~T();
		}

	public:

		Modifier& operator=(const Modifier& other) noexcept
		{
			m_name = other.m_name;
			m_type = other.m_type;
			switch (m_type)
			{
			case Type::STRING:        new(m_BUFFER) UnicodeString(other.cast<UnicodeString>()); break;
			case Type::STRING_NOCASE: new(m_BUFFER) std::u32string(other.cast<std::u32string>()); break;
			default:
				memcpy(m_BUFFER, other.m_BUFFER, sizeof(m_BUFFER));
				break;
			}
			return *this;
		}

		Modifier& operator=(Modifier&& other) noexcept
		{
			m_name = other.m_name;
			m_type = other.m_type;
			memcpy(m_BUFFER, other.m_BUFFER, sizeof(m_BUFFER));
			other.m_type = Type::NONE;
			return *this;
		}

		~Modifier() noexcept
		{
			if (m_type == Type::STRING)
				destruct<UnicodeString>();
			else if (m_type == Type::STRING_NOCASE)
				destruct<std::u32string>();
		}

		constexpr std::string_view getName() const noexcept { return m_name; }

	private:
		template <class T>
		bool validateType() const noexcept
		{
			if constexpr (std::is_same_v<T, UnicodeString>)       return m_type == Type::STRING;
			else if constexpr (std::is_same_v<T, std::u32string>) return m_type == Type::STRING_NOCASE;
			else if constexpr (std::is_same_v<T, uint32_t>)       return m_type == Type::UINT32;
			else if constexpr (std::is_same_v<T, int32_t>)        return m_type == Type::INT32;
			else if constexpr (std::is_same_v<T, uint16_t>)       return m_type == Type::UINT16;
			else if constexpr (std::is_same_v<T, float>)          return m_type == Type::FLOAT;
			else if constexpr (std::is_same_v<T, bool>)           return m_type == Type::BOOL;
			else if constexpr (std::is_same_v<T, float[2]>)       return m_type == Type::FLOATARRAY;
			else
				return false;
		}
	public:

		template <class T>
		T& getValue()
		{
			if (!validateType<T>())
				throw std::runtime_error("Template type does match internal type");

			return cast<T>();
		}

		template <class T>
		const T& getValue() const
		{
			if (!validateType<T>())
				throw std::runtime_error("Template type does match internal type");

			return cast<T>();
		}

		template <class T>
		Modifier& operator=(const T& value)
		{
			if (!validateType<T>())
				throw std::runtime_error("Template type does match internal type");

			cast<T>() = value;
			return *this;
		}

		void write_cht(std::fstream& outFile) const;

		template <class T>
		void writeVal(std::fstream& outFile) const
		{
			if      constexpr (std::is_same_v<T, bool>)           outFile << m_name << " = " << std::boolalpha << cast<T>() << '\n';
			else if constexpr (std::is_same_v<T, float[2]>)       outFile << m_name << " = " << cast<T>()[0] << ' ' << cast<T>()[1] << '\n';
			else if constexpr (std::is_same_v<T, std::u32string>) outFile << m_name << " = " << UnicodeString::U32ToStr(cast<T>()) << '\n';
			else                                                  outFile << m_name << " = " << cast<T>() << '\n';
		}

		void write_ini(std::fstream& outFile) const;
	};
};
