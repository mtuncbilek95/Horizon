#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Name", false]);
	class H_EXPORT NameComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(NameComponent);
	public:
		NameComponent() = default;
		~NameComponent() = default;

		HFIELD();
		std::string m_name;
	};
}