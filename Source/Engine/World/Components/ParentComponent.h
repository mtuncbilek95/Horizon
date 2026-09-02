#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/Containers/NameId.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["ParentComponent", "Socket"]);
	class H_EXPORT ParentComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(ParentComponent);
	public:
		ParentComponent() = default;
		~ParentComponent() = default;

		HFIELD();
		EntityHandle m_id;
	};
}