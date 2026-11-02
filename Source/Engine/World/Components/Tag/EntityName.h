#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["EntityName", "Tag"]);
	class EntityName final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(EntityName);
	public:
		EntityName() = default;
		~EntityName() = default;

		HFIELD();
		std::string m_name;
	};
}