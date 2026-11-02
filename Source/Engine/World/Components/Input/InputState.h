#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["InputState", "Input"]);
	class InputState final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(InputState);
	public:
		InputState() = default;
		~InputState() = default;

		// List<Engine::InputActionState> m_actions;
	};
}