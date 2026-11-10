#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RHI/Common/SampleCount.h>

#include <Engine/Rendering/Settings/AliasingType.h>

namespace Horizon::Rendering
{
	HCLASS();
	class AntiAliasingSettings : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(AntiAliasingSettings);
	public:
		HFIELD();
		AliasingType m_type = AliasingType::FXAA;

		HFIELD();
		RHI::SampleCount m_sampleCount = RHI::SampleCount::Count1;
	};
}