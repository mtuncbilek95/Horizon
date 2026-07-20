#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <vector>

namespace Horizon::Reflect
{
	struct ArrayOps
	{
		usize(*count)(const void*);
		void* (*element)(void*, usize);
		const void* (*elementConst)(const void*, usize);
		void (*resize)(void*, usize);
	};

	template<typename E>
	const ArrayOps* VectorOpsFor()
	{
		static const ArrayOps ops{
			[](const void* v) -> usize { return static_cast<const std::vector<E>*>(v)->size(); },
			[](void* v, usize i) -> void* { return &(*static_cast<std::vector<E>*>(v))[i]; },
			[](const void* v, usize i) -> const void* { return &(*static_cast<const std::vector<E>*>(v))[i]; },
			[](void* v, usize n) { static_cast<std::vector<E>*>(v)->resize(n); }
		};
		return &ops;
	}
}