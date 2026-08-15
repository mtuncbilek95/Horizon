#pragma once

#include <Runtime/RTTR/TypeHandle.h>

namespace Horizon::Reflect
{
	class H_EXPORT Base
	{
	public:
		virtual ~Base() = default;

		virtual TypeHandle GetTypeId() const = 0;
	};
}