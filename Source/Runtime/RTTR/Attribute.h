#pragma once

#include <Runtime/RTTR/TypeHandle.h>

namespace Horizon::Reflect
{
	class Attribute
	{
	public:
		virtual ~Attribute() = default;

		virtual TypeHandle GetTypeId() const = 0;
	};
}