#pragma once

#include <Runtime/Reflection/Type.h>

namespace Horizon
{
	class TypeAttribute
	{
	public:
		virtual ~TypeAttribute() = default;

		virtual ReflectionTypeHandle GetTypeId() const = 0;
	};
}