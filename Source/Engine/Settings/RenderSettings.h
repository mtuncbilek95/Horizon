#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <Engine/Rendering/Settings/AntiAliasingSettings.h>

namespace Horizon::Engine
{
	HCLASS();
	class RenderSettings : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(RenderSettings);
	public:
		HFIELD();
		Rendering::AntiAliasingSettings m_antiAliasing;
	};
}