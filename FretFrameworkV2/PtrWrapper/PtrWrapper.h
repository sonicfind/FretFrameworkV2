#pragma once

template <class T>
class PointerWrapper
{
	T* ptr = nullptr;

public:
	PointerWrapper() = default;
	PointerWrapper(T& obj) : ptr(&obj) {}
	T* operator->() noexcept { return ptr; }
	T& operator*() noexcept { return *ptr; }
	const T* operator->() const noexcept { return ptr; }
	const T& operator*() const noexcept { return *ptr; }
	operator bool() { return ptr != nullptr; }
	friend auto operator<=>(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs <=> *rhs;
	}
	friend bool operator==(const PointerWrapper& lhs, const PointerWrapper& rhs)
	{
		return *lhs == *rhs;
	}
};


