#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["MeshComponent", "Rendering"]);
	class H_EXPORT MeshComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(MeshComponent);
	public:
		MeshComponent() = default;
		~MeshComponent() = default;
	};
}