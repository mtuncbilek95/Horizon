#pragma once

#include <Engine/Rendering/Renderables/RenderablePointLight.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["PointLightBatch", "Benchmark"]);
	class PointLightBatch final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(PointLightBatch);
	public:
		PointLightBatch() = default;
		~PointLightBatch() = default;

		List<Engine::RenderablePointLight> m_lights;
	};
}