#pragma once

template <class T>
class PointerWrapper
{
	T* ptr = nullptr;

public:
	PointerWrapper() = default;
	PointerWrapper(T& obj) : ptr(&obj) {}
	PointerWrapper& operator=(const PointerWrapper&) = default;
	PointerWrapper& operator=(T& obj) { ptr = &obj; return *this; }
	PointerWrapper& operator=(std::nullptr_t) { ptr = nullptr; return *this; }
	T* operator->() const noexcept { return ptr; }
	T& operator*() const noexcept { return *ptr; }
	operator bool() { return ptr != nullptr; }

	T* raw() const noexcept { return ptr; };

	friend auto operator<=>(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs <=> *rhs;
	}
	friend bool operator==(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs == *rhs;
	}
};

template <class T>
class PointerWrapper<const T>
{
	const T* ptr = nullptr;

public:
	PointerWrapper() = default;
	PointerWrapper(T& obj) : ptr(&obj) {}
	PointerWrapper(const T& obj) : ptr(&obj) {}
	PointerWrapper& operator=(const PointerWrapper&) = default;
	PointerWrapper& operator=(T& obj) { ptr = &obj; return *this; }
	PointerWrapper& operator=(const T& obj) { ptr = &obj; return *this; }
	PointerWrapper& operator=(std::nullptr_t) { ptr = nullptr; return *this; }
	const T* operator->() const noexcept { return ptr; }
	const T& operator*() const noexcept { return *ptr; }
	operator bool() { return ptr != nullptr; }

	const T* raw() const noexcept { return ptr; };

	friend auto operator<=>(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs <=> *rhs;
	}
	friend bool operator==(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs == *rhs;
	}
};
