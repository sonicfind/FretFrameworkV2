#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
template <class T>
auto& GetNode(SimpleFlatMap<T>& map, uint64_t position)
{
	auto iter = map.end();
	while (iter != map.begin())
	{
		--iter;
		if (iter->key <= position)
			break;
	}
	return *iter;
}
