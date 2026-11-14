#include "PhysicsWriteBack.h"

#include <Engine/Physics/PhysicsService.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Physics/PhysicsBody.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>
#include <Engine/World/Systems/Physics/PhysicsStep.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Math/Quat.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace Horizon::World::System
{
	b8 PhysicsWriteBack::OnInitialize()
	{
		m_physics = GetEngine()->RequestService<Engine::PhysicsService>();
		if (!m_physics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsService is missing, simulated transforms cannot be written back");
			return false;
		}

		return true;
	}

	void PhysicsWriteBack::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		JPH::PhysicsSystem* pWorld = m_physics->GetWorld();
		const JPH::BodyInterface& bodies = pWorld->GetBodyInterfaceNoLock();

		JPH::BodyIDVector active;
		pWorld->GetActiveBodies(JPH::EBodyType::RigidBody, active);

		for (const JPH::BodyID& id : active)
		{
			if (bodies.GetMotionType(id) != JPH::EMotionType::Dynamic)
				continue;

			Engine::EntityHandle entity;
			entity.index = bodies.GetUserData(id);

			Component::LocalTransform* pLocal = currentScene.FindComponent<Component::LocalTransform>(entity);
			if (!pLocal)
				continue;

			JPH::RVec3 position;
			JPH::Quat rotation;
			bodies.GetPositionAndRotation(id, position, rotation);

			pLocal->m_position = Math::Vec3f(position.GetX(), position.GetY(), position.GetZ());
			pLocal->m_rotation = Math::Quat(rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()).ToEuler();
		}
	}

	void PhysicsWriteBack::OnFinalize()
	{
	}

	void PhysicsWriteBack::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::PhysicsBody>();
		graph.Writes<Component::LocalTransform>();

		graph.RunsAfter<PhysicsStep>();
		graph.RunsBefore<LocalToWorld>();
	}
}