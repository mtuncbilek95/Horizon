#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <Engine/Settings/RenderSettings.h>

namespace Horizon::Engine
{
	HCLASS();
	class EngineSettings : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(EngineSettings);
	public:
		HFIELD();
		RenderSettings m_render;
	};
}