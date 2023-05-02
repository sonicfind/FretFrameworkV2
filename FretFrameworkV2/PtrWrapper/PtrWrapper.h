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
	T* operator->() noexcept { return ptr; }
	T& operator*() noexcept { return *ptr; }
	const T* operator->() const noexcept { return ptr; }
	const T& operator*() const noexcept { return *ptr; }
	operator bool() { return ptr != nullptr; }

	T* raw() noexcept { return ptr; };
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


