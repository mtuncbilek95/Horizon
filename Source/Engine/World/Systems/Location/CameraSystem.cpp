#include "CameraSystem.h"

#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/LocalToWorldComponent.h>
#include <Engine/World/Components/CameraMatrixComponent.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	b8 CameraSystem::OnInitialize()
	{
		return true;
	}

	void CameraSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		const f32 h = f32(m_workableArea.Y());
		if (h <= 0.0f) 
			return;

		const f32 aspect = f32(m_workableArea.X()) / h;

		currentScene.ForEach<CameraComponent, LocalToWorldComponent, CameraMatrixComponent>([&](EntityHandle handl, CameraComponent& camera,
			LocalToWorldComponent& worldMat, CameraMatrixComponent& cameraMatrix)
			{
				const Math::Mat4f view = worldMat.m_worldMatrix.Inverse();
				const Math::Mat4f proj = Math::Mat4f::Perspective(
					Math::DegToRad(camera.m_fov), aspect, camera.m_nearPlane, camera.m_farPlane);

				cameraMatrix.m_view = view;
				cameraMatrix.m_projection = proj;
				cameraMatrix.m_viewProjection = proj * view;
				cameraMatrix.m_worldPosition = worldMat.m_worldMatrix.GetTranslation();
			});
	}

	void CameraSystem::OnFinalize()
	{
	}
}