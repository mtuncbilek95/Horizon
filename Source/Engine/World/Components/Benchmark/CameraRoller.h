#pragma once

#include <Engine/Rendering/Renderables/RenderablePointLight.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/Vector/List.h>
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


	};
}