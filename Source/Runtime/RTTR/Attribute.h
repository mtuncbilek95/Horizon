#pragma once

#include <Runtime/RTTR/TypeHandle.h>

namespace Horizon::Reflect
{
	class RUNTIME_API Attribute
	{
	public:
		virtual ~Attribute() = default;

		virtual TypeHandle GetTypeId() const = 0;
	};
}