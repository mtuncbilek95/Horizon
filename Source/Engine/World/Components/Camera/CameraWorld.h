#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["CameraWorld", "Camera"]);
	class CameraWorld final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraWorld);
	public:
		CameraWorld() = default;
		~CameraWorld() = default;

		Math::Mat4f m_view;
		Math::Mat4f m_projection;
		Math::Mat4f m_viewProjection;
		Math::Vec3f m_worldPosition;
	};
}