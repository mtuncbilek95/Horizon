#include "PhysicsBodySync.h"

#include <Engine/Physics/PhysicsLayers.h>
#include <Engine/Physics/PhysicsService.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Physics/Collider.h>
#include <Engine/World/Components/Physics/PhysicsBody.h>
#include <Engine/World/Components/Physics/RigidBody.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Math/Quat.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <algorithm>

namespace Horizon::World::System
{
	namespace
	{
		constexpr u64 kHashSeed = 14695981039346656037ull;
		constexpr u64 kHashPrime = 1099511628211ull;

		template<typename T>
		u64 HashValue(u64 hash, const T& value)
		{
			const u8* pBytes = reinterpret_cast<const u8*>(&value);

			for (usize i = 0; i < sizeof(T); i++)
			{
				hash ^= pBytes[i];
				hash *= kHashPrime;
			}

			return hash;
		}

		u64 HashShape(const Component::Collider& collider)
		{
			u64 hash = kHashSeed;
			hash = HashValue(hash, collider.m_shape);
			hash = HashValue(hash, collider.m_halfExtents);
			hash = HashValue(hash, collider.m_radius);
			hash = HashValue(hash, collider.m_halfHeight);
			hash = HashValue(hash, collider.m_offset);
			return hash;
		}

		u64 HashSettings(const Component::RigidBody& rigidBody)
		{
			u64 hash = kHashSeed;
			hash = HashValue(hash, rigidBody.m_motion);
			hash = HashValue(hash, rigidBody.m_mass);
			hash = HashValue(hash, rigidBody.m_friction);
			hash = HashValue(hash, rigidBody.m_restitution);
			hash = HashValue(hash, rigidBody.m_linearDamping);
			hash = HashValue(hash, rigidBody.m_angularDamping);
			hash = HashValue(hash, rigidBody.m_gravityFactor);
			hash = HashValue(hash, rigidBody.m_continuousCollision);
			hash = HashValue(hash, rigidBody.m_isTrigger);
			return hash;
		}

		JPH::EMotionType ToJolt(Engine::PhysicsMotion motion)
		{
			switch (motion)
			{
			case Engine::PhysicsMotion::Dynamic:
				return JPH::EMotionType::Dynamic;
			case Engine::PhysicsMotion::Kinematic:
				return JPH::EMotionType::Kinematic;
			default:
				return JPH::EMotionType::Static;
			}
		}

		JPH::ShapeSettings::ShapeResult CreateShape(const Component::Collider& collider)
		{
			JPH::ShapeSettings::ShapeResult result;

			switch (collider.m_shape)
			{
			case Engine::ColliderShape::Box:
			{
				const JPH::Vec3 halfExtents(collider.m_halfExtents.X(), collider.m_halfExtents.Y(), collider.m_halfExtents.Z());
				JPH::BoxShapeSettings settings(halfExtents, std::min(JPH::cDefaultConvexRadius, halfExtents.ReduceMin()));
				settings.SetEmbedded();
				result = settings.Create();
				break;
			}
			case Engine::ColliderShape::Sphere:
			{
				JPH::SphereShapeSettings settings(collider.m_radius);
				settings.SetEmbedded();
				result = settings.Create();
				break;
			}
			case Engine::ColliderShape::Capsule:
			{
				JPH::CapsuleShapeSettings settings(collider.m_halfHeight, collider.m_radius);
				settings.SetEmbedded();
				result = settings.Create();
				break;
			}
			case Engine::ColliderShape::Cylinder:
			{
				const f32 convexRadius = std::min(JPH::cDefaultConvexRadius, std::min(collider.m_halfHeight, collider.m_radius));
				JPH::CylinderShapeSettings settings(collider.m_halfHeight, collider.m_radius, convexRadius);
				settings.SetEmbedded();
				result = settings.Create();
				break;
			}
			}

			const b8 hasOffset = collider.m_offset.X() != 0.f || collider.m_offset.Y() != 0.f || collider.m_offset.Z() != 0.f;
			if (result.HasError() || !hasOffset)
				return result;

			JPH::RotatedTranslatedShapeSettings offsetSettings(JPH::Vec3(collider.m_offset.X(), collider.m_offset.Y(), collider.m_offset.Z()), JPH::Quat::sIdentity(), result.Get());
			offsetSettings.SetEmbedded();
			return offsetSettings.Create();
		}
	}

	b8 PhysicsBodySync::OnInitialize()
	{
		m_physics = GetEngine()->RequestService<Engine::PhysicsService>();
		if (!m_physics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsService is missing, bodies cannot be synchronized");
			return false;
		}

		return true;
	}

	void PhysicsBodySync::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		JPH::PhysicsSystem* pWorld = m_physics->GetWorld();
		JPH::BodyInterface& bodies = pWorld->GetBodyInterfaceNoLock();

		JPH::BodyIDVector existing;
		pWorld->GetBodies(existing);

		JPH::BodyIDVector stale;

		if (m_lastSceneId != currentScene.GetInstanceId())
		{
			stale = existing;
			m_lastSceneId = currentScene.GetInstanceId();
		}
		else
		{
			for (const JPH::BodyID& id : existing)
			{
				Engine::EntityHandle entity;
				entity.index = bodies.GetUserData(id);

				if (!currentScene.IsAlive(entity))
				{
					stale.push_back(id);
					continue;
				}

				const Component::PhysicsBody* pBody = currentScene.FindComponent<Component::PhysicsBody>(entity);
				const b8 owned = pBody && pBody->m_bodyId == id.GetIndexAndSequenceNumber()
					&& currentScene.HasComponent<Component::RigidBody>(entity)
					&& currentScene.HasComponent<Component::Collider>(entity);

				if (!owned)
					stale.push_back(id);
			}
		}

		if (!stale.empty())
		{
			bodies.RemoveBodies(stale.data(), static_cast<int>(stale.size()));
			bodies.DestroyBodies(stale.data(), static_cast<int>(stale.size()));
		}

		JPH::BodyIDVector replaced;
		JPH::BodyIDVector created;

		currentScene.ForEach<Component::LocalTransform, Component::RigidBody, Component::Collider, Component::PhysicsBody>(
			[&](Engine::EntityHandle entity, Component::LocalTransform& local, Component::RigidBody& rigidBody, Component::Collider& collider, Component::PhysicsBody& body)
			{
				const u64 settingsHash = HashSettings(rigidBody);
				const u64 shapeHash = HashShape(collider);

				const Math::Quat orientation = Math::Quat::FromEuler(local.m_rotation);
				const JPH::RVec3 position(local.m_position.X(), local.m_position.Y(), local.m_position.Z());
				const JPH::Quat rotation(orientation.X(), orientation.Y(), orientation.Z(), orientation.W());

				JPH::BodyID id(body.m_bodyId);

				if (body.m_bodyId != kInvalid32 && !bodies.IsAdded(id))
					body.m_bodyId = kInvalid32;

				if (body.m_bodyId != kInvalid32 && body.m_settingsHash != settingsHash)
				{
					replaced.push_back(id);
					body.m_bodyId = kInvalid32;
				}

				if (body.m_bodyId == kInvalid32)
				{
					JPH::ShapeSettings::ShapeResult shape = CreateShape(collider);
					if (shape.HasError())
					{
						Terminal::Error(StringHelper::GetTypeName(this), "Entity {} collider could not be created: {}", entity.Index(), shape.GetError().c_str());
						return;
					}

					const JPH::EMotionType motion = ToJolt(rigidBody.m_motion);
					const JPH::ObjectLayer layer = motion == JPH::EMotionType::Static ? Engine::PhysicsLayers::Static : Engine::PhysicsLayers::Moving;

					JPH::BodyCreationSettings settings(shape.Get(), position, rotation, motion, layer);
					settings.mFriction = rigidBody.m_friction;
					settings.mRestitution = rigidBody.m_restitution;
					settings.mLinearDamping = rigidBody.m_linearDamping;
					settings.mAngularDamping = rigidBody.m_angularDamping;
					settings.mGravityFactor = rigidBody.m_gravityFactor;
					settings.mIsSensor = rigidBody.m_isTrigger;
					settings.mUserData = entity.index;
					settings.mMotionQuality = rigidBody.m_continuousCollision ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;

					if (motion == JPH::EMotionType::Dynamic)
					{
						settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
						settings.mMassPropertiesOverride.mMass = rigidBody.m_mass;
					}

					JPH::Body* pBody = bodies.CreateBody(settings);
					if (!pBody)
					{
						Terminal::Error(StringHelper::GetTypeName(this), "Body limit reached, entity {} has no physics body", entity.Index());
						return;
					}

					created.push_back(pBody->GetID());
					body.m_bodyId = pBody->GetID().GetIndexAndSequenceNumber();
					body.m_settingsHash = settingsHash;
					body.m_shapeHash = shapeHash;
					return;
				}

				if (body.m_shapeHash != shapeHash)
				{
					JPH::ShapeSettings::ShapeResult shape = CreateShape(collider);
					if (shape.HasError())
					{
						Terminal::Error(StringHelper::GetTypeName(this), "Entity {} collider could not be rebuilt: {}", entity.Index(), shape.GetError().c_str());
						return;
					}

					bodies.SetShape(id, shape.Get(), true, JPH::EActivation::Activate);
					body.m_shapeHash = shapeHash;
				}

				if (rigidBody.m_motion == Engine::PhysicsMotion::Kinematic)
				{
					bodies.MoveKinematic(id, position, rotation, Engine::PhysicsService::FixedStep);
					return;
				}

				if (rigidBody.m_motion == Engine::PhysicsMotion::Static)
				{
					JPH::RVec3 currentPosition;
					JPH::Quat currentRotation;
					bodies.GetPositionAndRotation(id, currentPosition, currentRotation);

					if (!currentPosition.IsClose(position) || !currentRotation.IsClose(rotation))
						bodies.SetPositionAndRotation(id, position, rotation, JPH::EActivation::DontActivate);
				}
			});

		if (!replaced.empty())
		{
			bodies.RemoveBodies(replaced.data(), static_cast<int>(replaced.size()));
			bodies.DestroyBodies(replaced.data(), static_cast<int>(replaced.size()));
		}

		if (!created.empty())
		{
			JPH::BodyInterface::AddState state = bodies.AddBodiesPrepare(created.data(), static_cast<int>(created.size()));
			bodies.AddBodiesFinalize(created.data(), static_cast<int>(created.size()), state, JPH::EActivation::Activate);
		}
	}

	void PhysicsBodySync::OnFinalize()
	{
	}

	void PhysicsBodySync::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::LocalTransform>();
		graph.Reads<Component::RigidBody>();
		graph.Reads<Component::Collider>();
		graph.Writes<Component::PhysicsBody>();
	}
}