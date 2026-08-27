#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Rotation Component", "Location", true]);
	class H_EXPORT RotationComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(RotationComponent);
	public:
		RotationComponent() = default;
		~RotationComponent() = default;
	};
}