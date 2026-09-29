#pragma once

#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class PhysicsService;
	class RigidBodyComponent;
	class TransformComponent;

	HCLASS(SystemOrder[1000]);
	class H_EXPORT RigidBodySystem : public System
	{
		HORIZON_TYPE_REFLECT(RigidBodySystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::Physics; }

	private:
		void CreateBody(EntityHandle entity, TransformComponent& transform, RigidBodyComponent& rigidBody);
		void DestroyBody(u32 bodyId);
		void DestroyAllBodies();
		void CollectDeadBodies(Scene& currentScene);

	private:
		PhysicsService* m_physics = nullptr;

		std::unordered_map<u32, EntityHandle> m_bodies;
		f32 m_accumulator = 0.0f;
	};
}