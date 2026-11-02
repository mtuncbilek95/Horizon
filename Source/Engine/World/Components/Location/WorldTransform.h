#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Mat4f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["WorldTransform", "Location"]);
	class WorldTransform final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(WorldTransform);
	public:
		WorldTransform() = default;
		~WorldTransform() = default;

		Math::Mat4f m_worldMatrix;
	};
}