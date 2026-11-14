#include "CameraResolver.h"

#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Camera/CameraFollowState.h>
#include <Engine/World/Components/Camera/TargetFollower.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Horizon::World::System
{
	namespace
	{
		f32 DampFactor(f32 halfLife, f32 dt)
		{
			if (halfLife <= 0.f)
				return 1.f;

			return 1.f - std::exp2(-dt / halfLife);
		}

		Math::Vec3f RotateByYaw(const Math::Vec3f& value, f32 yaw)
		{
			const f32 c = std::cos(yaw);
			const f32 s = std::sin(yaw);

			return Math::Vec3f(value.X() * c + value.Z() * s, value.Y(), -value.X() * s + value.Z() * c);
		}

		f32 WrapAngle(f32 angle)
		{
			return std::remainder(angle, 2.f * std::numbers::pi_v<f32>);
		}
	}

	b8 CameraResolver::OnInitialize()
	{
		return true;
	}

	void CameraResolver::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		const f32 dt = ctx.DeltaTime();

		currentScene.ForEach<Component::LocalTransform, Component::TargetFollower, Component::CameraFollowState>(
			[&](Engine::EntityHandle entity, Component::LocalTransform& local, Component::TargetFollower& follower, Component::CameraFollowState& state)
			{
				if (!follower.m_target.IsValid() || follower.m_target == entity)
					return;

				const Component::LocalTransform* pTarget = currentScene.FindComponent<Component::LocalTransform>(follower.m_target);
				if (pTarget == nullptr)
					return;

				const f32 targetYaw = pTarget->m_rotation.Y();
				const Math::Vec3f desiredPivot = pTarget->m_position + RotateByYaw(follower.m_targetPivot, targetYaw);

				if (!state.m_initialized)
				{
					state.m_pivot = desiredPivot;
					state.m_alignYaw = targetYaw;
					state.m_initialized = true;
				}

				if (follower.m_autoAlignYaw)
					state.m_alignYaw += WrapAngle(targetYaw - state.m_alignYaw) * DampFactor(follower.m_autoAlignDamping, dt);
				else
					state.m_alignYaw = 0.f;

				state.m_yaw = state.m_alignYaw + follower.m_defaultYaw + state.m_yawOffset;
				state.m_pitch = follower.m_defaultPitch + state.m_pitchOffset;
				state.m_distance = follower.m_defaultDistance + state.m_distanceOffset;

				if (follower.m_minPitch < follower.m_maxPitch)
					state.m_pitch = std::clamp(state.m_pitch, follower.m_minPitch, follower.m_maxPitch);

				if (follower.m_minDistance < follower.m_maxDistance)
					state.m_distance = std::clamp(state.m_distance, follower.m_minDistance, follower.m_maxDistance);

				const Math::Vec3f delta = desiredPivot - state.m_pivot;
				const f32 length = std::sqrt(delta.X() * delta.X() + delta.Y() * delta.Y() + delta.Z() * delta.Z());

				if (length > follower.m_deadZone)
				{
					const Math::Vec3f goal = state.m_pivot + delta * ((length - follower.m_deadZone) / length);
					const f32 horizontal = DampFactor(follower.m_positionDamping, dt);
					const f32 vertical = DampFactor(follower.m_verticalDamping, dt);

					state.m_pivot.X() += (goal.X() - state.m_pivot.X()) * horizontal;
					state.m_pivot.Y() += (goal.Y() - state.m_pivot.Y()) * vertical;
					state.m_pivot.Z() += (goal.Z() - state.m_pivot.Z()) * horizontal;
				}

				state.m_collisionDistance = state.m_distance;

				const f32 cosPitch = std::cos(state.m_pitch);
				const f32 sinPitch = std::sin(state.m_pitch);
				const Math::Vec3f back = Math::Vec3f(cosPitch * std::sin(state.m_yaw), -sinPitch, cosPitch * std::cos(state.m_yaw));

				local.m_position = state.m_pivot + back * state.m_collisionDistance + RotateByYaw(follower.m_shoulderOffset, state.m_yaw);
				local.m_rotation = Math::Vec3f(state.m_pitch, state.m_yaw, 0.f);
			});
	}

	void CameraResolver::OnFinalize()
	{
	}

	void CameraResolver::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::TargetFollower>();
		graph.Writes<Component::CameraFollowState>();
		graph.Writes<Component::LocalTransform>();

		graph.RunsBefore<LocalToWorld>();
	}
}