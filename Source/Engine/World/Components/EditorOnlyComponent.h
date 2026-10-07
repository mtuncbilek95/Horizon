#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["", ""]);
	class EditorOnlyComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(EditorOnlyComponent);
	public:
		EditorOnlyComponent() = default;
		~EditorOnlyComponent() = default;
	};
}