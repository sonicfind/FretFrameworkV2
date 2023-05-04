#pragma once
#include <vector>
#include <stdexcept>
#include "PtrWrapper/PtrWrapper.h"

template <typename T, typename Key = uint32_t>
class SimpleFlatMap
{
	static constexpr T BASE;
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

	std::vector<Node> m_list;
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

	PointerWrapper<T> try_emplace_back(Key key, const T& obj = BASE)
	{
		if (!m_list.empty() && m_list.back().key >= key)
			return {};

		return emplace_back(key, obj);
	}

	[[nodiscard]] T& get_or_emplace_back(Key key)
	{
		if (m_list.empty() || m_list.back().key < key)
			return *m_list.emplace_back(key, BASE);
		return *m_list.back();
	}

	T& get_or_emplaceNodeFromBack(uint32_t position)
	{
		if (isEmpty())
			return emplace_back(position);

		auto iter = iterFromBack(position);
		if (iter == end() || iter->key > position)
			iter = emplace(iter, position);
		return **iter;
	}

	auto emplace(typename std::vector<Node>::iterator _Where, uint32_t position, const T& obj = BASE)
	{
		return m_list.emplace(_Where, position, obj);
	}

	auto erase(typename std::vector<Node>::iterator _Where)
	{
		return m_list.erase(_Where);
	}

	[[nodiscard]] T& operator[](Key key)
	{
		auto iter = std::lower_bound(begin(), end(), key);
		if (iter == end() || key < iter->key)
			iter = m_list.emplace(iter, key, BASE);
		return iter->object;
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

	// Assumes key exists
	T& getNodeFromBack(uint32_t position)
	{
		return **iterFromBack(position);
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
	auto iterFromBack(uint32_t position)
	{
		auto iter = end();
		while (iter != begin())
		{
			--iter;
			if (iter->key <= position)
				break;
		}
		return iter;
	}
};

