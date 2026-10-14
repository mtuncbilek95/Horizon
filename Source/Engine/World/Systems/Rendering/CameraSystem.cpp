#include "CameraSystem.h"

#include <Engine/World/Components/Rendering/CameraComponent.h>
#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Mat4f.h>

namespace Horizon::Engine
{
	b8 CameraSystem::OnInitialize()
	{
		return true;
	}

	void CameraSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		m_active = ResolveActiveCamera(currentScene);

		currentScene.ForEach<CameraComponent, TransformComponent>([&](EntityHandle handl, CameraComponent& camera,
			TransformComponent& worldMat)
			{
				const Math::Mat4f view = worldMat.m_worldMatrix.Inverse();
				const Math::Mat4f proj = Math::Mat4f::PerspectiveRZ(camera.m_fov, camera.m_targetScreen.X() / camera.m_targetScreen.Y(),
					camera.m_nearPlane, camera.m_farPlane);

				camera.m_view = view;
				camera.m_projection = proj;
				camera.m_viewProjection = proj * view;
				camera.m_worldPosition = worldMat.m_worldMatrix.GetTranslation();
			});
	}

	void CameraSystem::OnFinalize()
	{
	}

	void CameraSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	EntityHandle CameraSystem::ResolveActiveCamera(Scene& currentScene) const
	{
		if (m_override.IsValid() && currentScene.IsAlive(m_override) && currentScene.HasComponent<CameraComponent>(m_override))
			return m_override;

		EntityHandle bestCamera = {};
		i32 bestPriority = 0;

		currentScene.ForEach<CameraComponent>([&](EntityHandle handl, CameraComponent& camera)
			{
				if (currentScene.HasComponent<EditorOnlyComponent>(handl))
					return;

				if (bestCamera.IsValid() && camera.m_priority <= bestPriority)
					return;

				bestCamera = handl;
				bestPriority = camera.m_priority;
			});

		return bestCamera;
	}
}