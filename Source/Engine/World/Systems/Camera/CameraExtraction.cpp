#include "CameraExtraction.h"

#include <Engine/World/SystemDependency.h>

#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Camera/PerspectiveCamera.h>
#include <Engine/World/Components/Location/WorldTransform.h>

#include <Engine/World/Systems/Location/LocalToWorld.h>

namespace Horizon::World::System
{
	b8 CameraExtraction::OnInitialize()
	{
		m_present = GetEngine()->RequestService<Engine::PresentationService>();

		if (!m_present)
			return false;

		return true;
	}

	void CameraExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		currentScene.ForEach<Component::WorldTransform, Component::PerspectiveCamera, Component::CameraWorld>(
			[&](Engine::EntityHandle handl, Component::WorldTransform& world, Component::PerspectiveCamera& perspective, Component::CameraWorld& viewProj)
			{
				Math::Vec2u viewportSize = m_present->GetSceneExtent();
				const Math::Mat4f view = world.m_worldMatrix.Inverse();
				const Math::Mat4f proj = Math::Mat4f::PerspectiveRZ(perspective.m_fov, (f32)viewportSize.X() / (f32)viewportSize.Y(),
					perspective.m_nearPlane, perspective.m_farPlane);

				viewProj.m_view = view;
				viewProj.m_projection = proj;
				viewProj.m_viewProjection = proj * view;
				viewProj.m_worldPosition = world.m_worldMatrix.GetTranslation();
			});
	}

	void CameraExtraction::OnFinalize()
	{
		m_present = nullptr;
	}

	void CameraExtraction::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::WorldTransform>();
		graph.Reads<Component::PerspectiveCamera>();
		graph.Writes<Component::CameraWorld>();

		graph.RunsAfter<LocalToWorld>();
	}
}