#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["EditorOnly", "Tag"]);
	class EditorOnly final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(EditorOnly);
	public:
		EditorOnly() = default;
		~EditorOnly() = default;
	};
}