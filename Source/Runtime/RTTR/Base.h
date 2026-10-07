#pragma once

#include <Runtime/RTTR/TypeHandle.h>

namespace Horizon::Reflect
{
	class RUNTIME_API Base
	{
	public:
		virtual ~Base() = default;

		virtual TypeHandle GetTypeId() const = 0;
	};
}