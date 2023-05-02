#pragma once
#include <fstream>
#include "Types/UnicodeString.h"

namespace Modifiers
{
	enum class Type
	{
		STRING,
		STRING_NOCASE,
		UINT32,
		INT32,
		UINT16,
		INT16,
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

		template <typename T>
		constexpr Modifier(std::string_view name, T&& value) : m_name(name)
		{
			using base_T = std::remove_const_t<std::remove_reference_t<T>>;
			static_assert (std::is_same<base_T, std::u32string>::value ||
				           std::is_same<base_T, UnicodeString>::value ||
				           std::is_same<base_T, uint32_t>::value ||
				           std::is_same<base_T, int32_t>::value ||
				           std::is_same<base_T, uint16_t>::value ||
				           std::is_same<base_T, int16_t>::value ||
				           std::is_same<base_T, bool>::value ||
				           std::is_same<base_T, float>::value, "Type is not allowed in a modifier");

			if constexpr (std::is_same<base_T, std::u32string>::value)
				m_type = Type::STRING_NOCASE;
			else if constexpr (std::is_same<base_T, UnicodeString>::value)
				m_type = Type::STRING;
			else if constexpr (std::is_same<base_T, uint32_t>::value)
				m_type = Type::UINT32;
			else if constexpr (std::is_same<base_T, int32_t>::value)
				m_type = Type::INT32;
			else if constexpr (std::is_same<base_T, uint16_t>::value)
				m_type = Type::UINT16;
			else if constexpr (std::is_same<base_T, uint16_t>::value)
				m_type = Type::INT16;
			else if constexpr (std::is_same<base_T, bool>::value)
				m_type = Type::BOOL;
			else
				m_type = Type::FLOAT;

			new(m_BUFFER) base_T(std::forward<T>(value));
		}

		constexpr Modifier(std::string_view name, float flt1, float flt2) : m_name(name), m_type(Type::FLOATARRAY)
		{
			new(m_BUFFER) float[2](flt1, flt2);
		}

		Modifier(const Modifier& other) noexcept : m_name(other.m_name), m_type(other.m_type)
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

		Modifier& operator=(const Modifier& other) noexcept
		{
			clear();
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
			clear();
			m_name = other.m_name;
			m_type = other.m_type;
			memcpy(m_BUFFER, other.m_BUFFER, sizeof(m_BUFFER));
			other.m_type = Type::NONE;
			return *this;
		}

		~Modifier() noexcept
		{
			clear();
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

		void clear()
		{
			if (m_type == Type::STRING)
				destruct<UnicodeString>();
			else if (m_type == Type::STRING_NOCASE)
				destruct<std::u32string>();
		}

	public:
		constexpr std::string_view getName() const noexcept { return m_name; }
		constexpr Type getType() const noexcept { return m_type; }

	private:
		template <class T>
		bool validateType() const noexcept
		{
			if constexpr (std::is_same_v<T, UnicodeString>)       return m_type == Type::STRING;
			else if constexpr (std::is_same_v<T, std::u32string>) return m_type == Type::STRING_NOCASE;
			else if constexpr (std::is_same_v<T, uint32_t>)       return m_type == Type::UINT32;
			else if constexpr (std::is_same_v<T, int32_t>)        return m_type == Type::INT32;
			else if constexpr (std::is_same_v<T, uint16_t>)       return m_type == Type::UINT16;
			else if constexpr (std::is_same_v<T, int16_t>)        return m_type == Type::INT16;
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
	};
};
