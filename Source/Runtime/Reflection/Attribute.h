#pragma once

#include <Runtime/Reflection/Class.h>

namespace Horizon
{
	class H_EXPORT Attribute : public Class 
	{
	public:
		Attribute() = default;
		virtual ~Attribute() = default;
	};
}