#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Mat4f.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["CameraMatrixComponent", "Location"]);
	class H_EXPORT CameraMatrixComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraMatrixComponent);
	public:
		CameraMatrixComponent() = default;
		~CameraMatrixComponent() = default;

		Math::Mat4f m_viewProjection;
		Math::Mat4f m_view;
		Math::Mat4f m_projection;
		Math::Vec3f m_worldPosition;
	};
}