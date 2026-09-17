#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT AssetObject : public Reflect::Base
	{
	public:
		virtual ~AssetObject() = default;
	};
}