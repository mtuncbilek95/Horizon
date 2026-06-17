#pragma once

#include <Runtime/Reflection/Definitions.h>

namespace Horizon
{
	class Type;

	class Class
	{
	public:
		virtual Type* GetType() const noexcept { return nullptr; }
	};
}