#include "TransformSystem.h"

#include <Engine/World/Components/TransformComponent.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	b8 TransformSystem::OnInitialize()
	{
		return true;
	}

	void TransformSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		currentScene.ForEach<TransformComponent>([&](EntityHandle handl, TransformComponent& transform)
			{
				transform.m_worldMatrix = Math::Mat4f::TRS(transform.m_position, Math::Quat::FromEuler(transform.m_rotation), transform.m_scale);
			});
	}

	void TransformSystem::OnFinalize()
	{
	}
}