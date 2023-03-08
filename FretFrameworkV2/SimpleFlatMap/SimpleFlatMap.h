#pragma once
#include <vector>
#include <stdexcept>

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

	[[nodiscard]] auto getIterator(Key key) noexcept
	{
		return std::lower_bound(begin(), end(), key);
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

	T* try_emplace_back(Key key)
	{
		if (!m_list.empty() && m_list.back().key >= key)
			return nullptr;

		return &m_list.emplace_back(key, BASE).object;
	}

	[[nodiscard]] T& emplace_back(Key key)
	{
		if (T* obj = try_emplace_back(key))
			return *obj;
		throw std::runtime_error("Can not add node at position to the back");
	}

	[[nodiscard]] T& get_or_emplace_back(Key key)
	{
		if (m_list.empty() || m_list.back().key < key)
			return *m_list.emplace_back(key, BASE);
		return *m_list.back();
	}

	[[nodiscard]] T& operator[](Key key)
	{
		auto iter = getIterator(key);
		if (iter == end() || key < iter->key)
			return m_list.emplace(iter, key, BASE)->object;
		return iter->object;
	}

	[[nodiscard]] T& at(Key key)
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return iter->object;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] const T& at(Key key) const
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return iter->object;
		throw std::runtime_error("Object at key does not exist");
	}

	[[nodiscard]] T* at_pointer(Key key) noexcept
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return &iter->object;
		return nullptr;
	}

	[[nodiscard]] const T* at_pointer(Key key) const noexcept
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return &iter->object;
		return nullptr;
	}

	[[nodiscard]] Node& at_node(Key key)
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return *iter;
		throw std::runtime_error("Node does not exist");
	}

	[[nodiscard]] const Node& at_node(Key key) const
	{
		auto iter = getIterator(key);
		if (iter != end() && key == iter->key)
			return *iter;
		throw std::runtime_error("Node does not exist");
	}

	[[nodiscard]] T& back()
	{
		return *m_list.back();
	}

	[[nodiscard]] const T& back() const
	{
		return *m_list.back();
	}

	[[nodiscard]] T* try_back(Key keytoValidate)
	{
		Node& node = m_list.back();
		if (node.key == keytoValidate)
			return &node.object;
		return nullptr;
	}

	[[nodiscard]] T* try_back(Key keytoValidate) const
	{
		const Node& node = m_list.back();
		if (node.key == keytoValidate)
			return &node.object;
		return nullptr;
	}

	[[nodiscard]] T& back(Key keytoValidate)
	{
		if (T* obj = try_back(keytoValidate))
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

	auto erase(typename std::vector<Node>::iterator _Where)
	{
		return m_list.erase(_Where);
	}

	auto emplace(typename std::vector<Node>::iterator _Where, uint32_t position)
	{
		return m_list.emplace(_Where, position, BASE);
	}

	T& getNodeFromBack(uint32_t position)
	{
		auto iter = end();
		while (iter != begin() && (iter - 1)->key >= position)
			--iter;

		if (iter == end() || iter->key > position)
			iter = emplace(iter, position);
		return **iter;
	}
};

