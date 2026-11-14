#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["CameraFollowState", "Camera"]);
	class CameraFollowState final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraFollowState);
	public:
		CameraFollowState() = default;
		~CameraFollowState() = default;

		Math::Vec3f m_pivot = Math::Vec3f::Zero();
		f32 m_alignYaw = 0.f;

		f32 m_yawOffset = 0.f;
		f32 m_pitchOffset = 0.f;
		f32 m_distanceOffset = 0.f;

		f32 m_yaw = 0.f;
		f32 m_pitch = 0.f;
		f32 m_distance = 0.f;
		f32 m_collisionDistance = 0.f;

		b8 m_initialized = false;
	};
}