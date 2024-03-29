#pragma once
#include <vector>
#include <stdexcept>
#include "PtrWrapper/PtrWrapper.h"

template <typename T, typename Key = uint64_t>
class SimpleFlatMap
{
	static constexpr T BASE{};
	struct Node
	{
		Key key;
		T object;

		auto operator<=>(const Key& key) const noexcept
		{
			return this->key <=> key;
		}

		T* operator->() noexcept { return &object; }
		const T* operator->() const noexcept { return &object; }
		T& operator*() noexcept { return object; }
		const T& operator*() const noexcept { return object; }
	};

public:
	SimpleFlatMap() = default;
	SimpleFlatMap(const SimpleFlatMap&) = default;
	SimpleFlatMap(SimpleFlatMap&&) = default;
	SimpleFlatMap& operator=(const SimpleFlatMap&) = default;
	SimpleFlatMap& operator=(SimpleFlatMap&&) = default;
	SimpleFlatMap(size_t spaceToReserve)
	{
		reserve(spaceToReserve);
	}

	void clear()
	{
		m_list.clear();
	}

	void reserve(size_t size)
	{
		m_list.reserve(size);
	}

	void shrink_to_fit()
	{
		m_list.shrink_to_fit();
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return m_list.size();
	}

	[[nodiscard]] size_t capacity() const noexcept
	{
		return m_list.capacity();
	}

	[[nodiscard]] bool isEmpty() const
	{
		return m_list.empty();
	}

	// Assumes correct placement
	T& emplace_back(Key key, const T& obj = BASE)
	{
		return m_list.emplace_back(key, obj).object;
	}

	T& get_or_emplace_back(Key key)
	{
		if (m_list.empty() || m_list.back().key < key)
			return *m_list.emplace_back(key, BASE);
		return *m_list.back();
	}

	void pop_back()
	{
		m_list.pop_back();
	}

	auto emplace(typename std::vector<Node>::iterator _Where, Key position, const T& obj = BASE)
	{
		return m_list.emplace(_Where, position, obj);
	}

	auto erase(typename std::vector<Node>::iterator _Where)
	{
		return m_list.erase(_Where);
	}

	[[nodiscard]] size_t find_or_emplace(size_t startingIndex, Key key)
	{
		return find_or_emplace_iter(startingIndex, key) - begin();
	}

	[[nodiscard]] size_t find_or_emplace(Key key)
	{
		return find_or_emplace(0, key);
	}

	[[nodiscard]] T& find_or_emplace_object(size_t startingIndex, Key key)
	{
		return find_or_emplace_iter(startingIndex, key)->object;
	}

	[[nodiscard]] T& operator[](Key key)
	{
		return find_or_emplace_object(0, key);
	}

	[[nodiscard]] T& at(Key key)
	{
		auto iter = std::lower_bound(begin(), end(), key);
		if (iter != end() && key == iter->key)
			return iter->object;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] const T& at(Key key) const
	{
		auto iter = std::lower_bound(begin(), end(), key);
		if (iter != end() && key == iter->key)
			return iter->object;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] bool contains(Key key) const noexcept
	{
		auto iter = std::lower_bound(begin(), end(), key);
		return iter != end() && key == iter->key;
	}

	[[nodiscard]] PointerWrapper<T> try_at(Key key) noexcept
	{
		auto iter = std::lower_bound(begin(), end(), key);
		if (iter != end() && key == iter->key)
			return iter->object;
		return nullptr;
	}

	[[nodiscard]] PointerWrapper<const T> try_at(Key key) const noexcept
	{
		auto iter = std::lower_bound(begin(), end(), key);
		if (iter != end() && key == iter->key)
			return iter->object;
		return nullptr;
	}

	[[nodiscard]] Node& at_index(size_t index)
	{
		return m_list[index];
	}

	[[nodiscard]] const Node& at_index(size_t index) const
	{
		return m_list[index];
	}

	[[nodiscard]] T& back()
	{
		return *m_list.back();
	}

	[[nodiscard]] const T& back() const
	{
		return *m_list.back();
	}

	[[nodiscard]] PointerWrapper<T> try_back(Key keytoValidate)
	{
		if (m_list.empty() || m_list.back().key != keytoValidate)
			return {};
		return m_list.back().object;
	}

	[[nodiscard]] PointerWrapper<const T> try_back(Key keytoValidate) const
	{
		if (m_list.empty() || m_list.back().key != keytoValidate)
			return {};
		return m_list.back().object;
	}

	[[nodiscard]] T& back(Key keytoValidate)
	{
		if (auto obj = try_back(keytoValidate))
			return *obj;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] const T& back(Key keytoValidate) const
	{
		if (T* obj = try_back(keytoValidate))
			return *obj;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] T& front()
	{
		return *m_list.front();
	}

	[[nodiscard]] const T& front() const
	{
		return *m_list.front();
	}

	[[nodiscard]] auto begin() noexcept { return m_list.begin(); }
	[[nodiscard]] auto begin() const noexcept { return m_list.begin(); }
	[[nodiscard]] auto end() noexcept { return m_list.end(); }
	[[nodiscard]] auto end() const noexcept { return m_list.end(); }

private:
	[[nodiscard]] auto find_or_emplace_iter(size_t startingIndex, Key key)
	{
		auto iter = std::lower_bound(begin() + startingIndex, end(), key);
		if (iter == end() || key < iter->key)
			iter = m_list.emplace(iter, key, BASE);
		return iter;
	}

private:
	std::vector<Node> m_list;
};

