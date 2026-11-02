#include "LocalToWorld.h"

#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Location/WorldTransform.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::World::System
{
	b8 LocalToWorld::OnInitialize()
	{
		return true;
	}

	void LocalToWorld::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		currentScene.ForEach<Component::LocalTransform, Component::WorldTransform>([&](Engine::EntityHandle handle, Component::LocalTransform& local, Component::WorldTransform& world) 
			{
				world.m_worldMatrix = Math::Mat4f::TRS(local.m_position, Math::Quat::FromEuler(local.m_rotation), local.m_scale);
			});
	}

	void LocalToWorld::OnFinalize()
	{
	}

	void LocalToWorld::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::LocalTransform>();
		graph.Writes<Component::WorldTransform>();
	}
}