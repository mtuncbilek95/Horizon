#pragma once

#include <Runtime/Definitions/ReflectionDefinitions.h>

namespace Horizon
{
	class H_EXPORT Class
	{
	public:
		Class() = default;
		virtual ~Class() = default;

		FORCEINLINE virtual Type* GetType() const noexcept { return nullptr; }
	};
}