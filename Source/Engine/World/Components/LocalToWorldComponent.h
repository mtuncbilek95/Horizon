#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Mat4f.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["LocalToWorldComponent", "Location"]);
	class H_EXPORT LocalToWorldComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(LocalToWorldComponent);
	public:
		LocalToWorldComponent() = default;
		~LocalToWorldComponent() = default;

		Math::Mat4f m_worldMatrix;
	};
}