#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class EngineSettings : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(EngineSettings);
	public:

	};
}