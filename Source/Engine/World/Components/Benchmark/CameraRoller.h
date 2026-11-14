#pragma once

#include <Engine/Rendering/Renderables/RenderablePointLight.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/UnitAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["CameraRoller", "Benchmark"]);
	class CameraRoller final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraRoller);
	public:
		CameraRoller() = default;
		~CameraRoller() = default;

		HFIELD(Unit["deg"], Degrees);
		f32 m_azimuth = 0;
		
		HFIELD(Unit["deg"], Degrees);
		f32 m_elevation = 0;

		HFIELD(Unit["meter"]);
		f32 m_distance = 0;

		HFIELD(Unit["deg/s"], Degrees);
		f32 m_rotationSpeed = 0;
	};
}