#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["NameComponent", "Tag"]);
	class H_EXPORT NameComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(NameComponent);
	public:
		NameComponent() = default;
		~NameComponent() = default;

		HFIELD();
		std::string m_name;
	};
}