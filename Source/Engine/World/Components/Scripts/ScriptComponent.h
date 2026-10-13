#pragma once

#include <Engine/Game/GameScript.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["ScriptComponent", "Script"]);
	class ENGINE_API ScriptComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(ScriptComponent);
	public:
		ScriptComponent() = default;
		~ScriptComponent();

		ScriptComponent(const ScriptComponent&) = delete;
		ScriptComponent& operator=(const ScriptComponent&) = delete;

		ScriptComponent(ScriptComponent&& other) noexcept;
		ScriptComponent& operator=(ScriptComponent&& other) noexcept;

		HFIELD();
		GameScript* m_script = nullptr;
	};
}