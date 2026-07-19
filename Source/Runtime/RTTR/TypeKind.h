#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Reflect
{
	enum class H_EXPORT TypeKind : u8
	{
		Object,
		Enum,
		Boolean,
		Char,
		Signed8,
		Signed16,
		Signed32,
		Signed64,
		Unsigned8,
		Unsigned16,
		Unsigned32,
		Unsigned64,
		Float32,
		Float64,
		String,
	};
}