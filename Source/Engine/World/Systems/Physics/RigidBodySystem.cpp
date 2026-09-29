#include "RigidBodySystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Physics/PhysicsService.h>
#include <Engine/Physics/PhysicsLayers.h>
#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/Physics/RigidBodyComponent.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec3f.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace Horizon::Engine
{
	static JPH::Vec3 ToJolt(const Math::Vec3f& v)
	{
		return JPH::Vec3(v.X(), v.Y(), v.Z());
	}

	static JPH::Quat ToJolt(const Math::Quat& q)
	{
		return JPH::Quat(q.X(), q.Y(), q.Z(), q.W());
	}

	static Math::Vec3f FromJolt(JPH::Vec3Arg v)
	{
		return Math::Vec3f(v.GetX(), v.GetY(), v.GetZ());
	}

	static Math::Quat FromJolt(JPH::QuatArg q)
	{
		return Math::Quat(q.GetX(), q.GetY(), q.GetZ(), q.GetW());
	}

	static JPH::EMotionType ToJolt(PhysicsMotion motion)
	{
		switch (motion)
		{
		case PhysicsMotion::Dynamic:
			return JPH::EMotionType::Dynamic;
		case PhysicsMotion::Kinematic:
			return JPH::EMotionType::Kinematic;
		default:
			return JPH::EMotionType::Static;
		}
	}

	b8 RigidBodySystem::OnInitialize()
	{
		m_physics = GetEngine()->RequestService<PhysicsService>();
		return m_physics != nullptr;
	}

	void RigidBodySystem::OnFinalize()
	{
		DestroyAllBodies();
	}

	void RigidBodySystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
		if (HasFlag(current, SystemGroup::Editor))
		{
			DestroyAllBodies();
			m_accumulator = 0.0f;
			return;
		}

		if (HasFlag(previous, SystemGroup::Editor) && HasFlag(current, SystemGroup::Physics))
		{
			Scene* pScene = GetWorldService()->GetCurrentWorld();
			if (!pScene)
				return;

			pScene->ForEach<TransformComponent, RigidBodyComponent>([&](EntityHandle entity, TransformComponent& transform, RigidBodyComponent& rigidBody)
				{
					CreateBody(entity, transform, rigidBody);
				});
		}
	}

	void RigidBodySystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		JPH::PhysicsSystem* pWorld = m_physics->GetWorld();
		if (!pWorld)
			return;

		JPH::BodyInterface& bodies = pWorld->GetBodyInterface();

		CollectDeadBodies(currentScene);

		currentScene.ForEach<TransformComponent, RigidBodyComponent>([&](EntityHandle entity, TransformComponent& transform, RigidBodyComponent& rigidBody)
			{
				if (rigidBody.m_bodyId == kInvalid32)
				{
					CreateBody(entity, transform, rigidBody);
					return;
				}

				if (rigidBody.m_motion == PhysicsMotion::Kinematic)
					bodies.MoveKinematic(JPH::BodyID(rigidBody.m_bodyId), ToJolt(transform.m_position), ToJolt(transform.m_orientation), PhysicsService::FixedStep);
			});

		m_accumulator += ctx.DeltaTime();

		u32 steps = (u32)(m_accumulator / PhysicsService::FixedStep);
		steps = (std::min)(steps, PhysicsService::MaxStepsPerFrame);

		if (steps == 0)
			return;

		m_accumulator -= steps * PhysicsService::FixedStep;
		if (m_accumulator > PhysicsService::FixedStep)
			m_accumulator = PhysicsService::FixedStep;

		const JPH::EPhysicsUpdateError error = pWorld->Update(steps * PhysicsService::FixedStep, (int)steps, m_physics->GetTempAllocator(), m_physics->GetJobBridge());
		if (error != JPH::EPhysicsUpdateError::None)
			Terminal::Warn(StringOps::GetName(this), "Jolt update reported error mask {:#x}", (u32)error);

		currentScene.ForEach<TransformComponent, RigidBodyComponent>([&](EntityHandle entity, TransformComponent& transform, RigidBodyComponent& rigidBody)
			{
				if (rigidBody.m_bodyId == kInvalid32 || rigidBody.m_motion != PhysicsMotion::Dynamic)
					return;

				const JPH::BodyID id(rigidBody.m_bodyId);
				if (!bodies.IsActive(id))
					return;

				JPH::RVec3 position;
				JPH::Quat rotation;
				bodies.GetPositionAndRotation(id, position, rotation);

				transform.m_position = FromJolt(position);
				transform.m_orientation = FromJolt(rotation);
			});
	}

	void RigidBodySystem::CreateBody(EntityHandle entity, TransformComponent& transform, RigidBodyComponent& rigidBody)
	{
		JPH::BodyInterface& bodies = m_physics->GetWorld()->GetBodyInterface();

		const Math::Vec3f& half = rigidBody.m_halfExtents;
		if (half.X() <= JPH::cDefaultConvexRadius || half.Y() <= JPH::cDefaultConvexRadius || half.Z() <= JPH::cDefaultConvexRadius)
		{
			Terminal::Warn(StringOps::GetName(this), "RigidBody half extents must exceed {} on every axis, body skipped", JPH::cDefaultConvexRadius);
			return;
		}

		JPH::BoxShapeSettings shapeSettings(ToJolt(half));
		JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
		if (shapeResult.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "Shape creation failed: {}", shapeResult.GetError().c_str());
			return;
		}

		const JPH::ObjectLayer layer = rigidBody.m_motion == PhysicsMotion::Static ? PhysicsLayers::Static : PhysicsLayers::Moving;

		JPH::BodyCreationSettings settings(shapeResult.Get(), ToJolt(transform.m_position), ToJolt(transform.m_orientation), ToJolt(rigidBody.m_motion), layer);

		if (rigidBody.m_mass > 0.0f)
		{
			settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
			settings.mMassPropertiesOverride.mMass = rigidBody.m_mass;
		}

		JPH::Body* pBody = bodies.CreateBody(settings);
		if (!pBody)
		{
			Terminal::Error(StringOps::GetName(this), "Jolt refused to create body, MaxBodies reached");
			return;
		}

		const JPH::EActivation activation = rigidBody.m_motion == PhysicsMotion::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
		bodies.AddBody(pBody->GetID(), activation);

		rigidBody.m_bodyId = pBody->GetID().GetIndexAndSequenceNumber();
		m_bodies[rigidBody.m_bodyId] = entity;
	}

	void RigidBodySystem::DestroyBody(u32 bodyId)
	{
		JPH::BodyInterface& bodies = m_physics->GetWorld()->GetBodyInterface();

		const JPH::BodyID id(bodyId);
		bodies.RemoveBody(id);
		bodies.DestroyBody(id);
	}

	void RigidBodySystem::DestroyAllBodies()
	{
		if (!m_physics || !m_physics->GetWorld())
			return;

		for (const auto& pair : m_bodies)
			DestroyBody(pair.first);

		m_bodies.clear();

		Scene* pScene = GetWorldService()->GetCurrentWorld();
		if (!pScene)
			return;

		pScene->ForEach<RigidBodyComponent>([](EntityHandle, RigidBodyComponent& rigidBody)
			{
				rigidBody.m_bodyId = kInvalid32;
			});
	}

	void RigidBodySystem::CollectDeadBodies(Scene& currentScene)
	{
		for (auto it = m_bodies.begin(); it != m_bodies.end();)
		{
			const EntityHandle entity = it->second;

			if (currentScene.IsAlive(entity) && currentScene.HasComponent<RigidBodyComponent>(entity))
			{
				++it;
				continue;
			}

			DestroyBody(it->first);
			it = m_bodies.erase(it);
		}
	}
}