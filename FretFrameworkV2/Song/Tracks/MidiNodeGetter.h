#pragma once
#include "SimpleFlatMap/SimpleFlatMap.h"
namespace Midi_Loader
{
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
}
