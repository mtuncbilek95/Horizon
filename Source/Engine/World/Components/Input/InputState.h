#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	struct InputActionState
	{
		Math::Vec2f value = Math::Vec2f(0.0f, 0.0f);
		b8 down = false;
		b8 pressed = false;
		b8 released = false;
	};

	HCLASS(Engine::ComponentId["InputState", "Input"]);
	class InputState final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(InputState);
	public:
		InputState() = default;
		~InputState() = default;

		List<InputActionState> m_actions;
	};
}
