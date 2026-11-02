#include "CameraExtraction.h"

#include <Engine/World/SystemDependency.h>

#include <Engine/World/Components/Camera/CameraViewport.h>
#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Camera/PerspectiveCamera.h>
#include <Engine/World/Components/Location/WorldTransform.h>

#include <Engine/World/Systems/Location/LocalToWorld.h>

namespace Horizon::World::System
{
	b8 CameraExtraction::OnInitialize()
	{
		return true;
	}

	void CameraExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		currentScene.ForEach<Component::WorldTransform, Component::CameraViewport, Component::PerspectiveCamera, Component::CameraWorld>(
			[&](Engine::EntityHandle handl, Component::WorldTransform& world, Component::CameraViewport& viewport, Component::PerspectiveCamera& perspective, Component::CameraWorld& viewProj)
			{
				const Math::Mat4f view = world.m_worldMatrix.Inverse();
				const Math::Mat4f proj = Math::Mat4f::PerspectiveRZ(perspective.m_fov, viewport.m_targetScreen.X() / viewport.m_targetScreen.Y(),
					perspective.m_nearPlane, perspective.m_farPlane);

				viewProj.m_view = view;
				viewProj.m_projection = proj;
				viewProj.m_viewProjection = proj * view;
				viewProj.m_worldPosition = world.m_worldMatrix.GetTranslation();
			}); 
	}

	void CameraExtraction::OnFinalize()
	{
	}

	void CameraExtraction::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::WorldTransform>();
		graph.Reads<Component::CameraViewport>();
		graph.Reads<Component::PerspectiveCamera>();
		graph.Writes<Component::CameraWorld>();

		graph.RunsAfter<LocalToWorld>();
	}
}