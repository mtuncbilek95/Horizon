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
				const Math::Vec3f target(0.f, 0.f, 0.f);
				const Math::Vec3f offset = world.m_worldMatrix.GetTranslation() - target;

				const f32 azimuth = (f32)ctx.GetTotalSeconds() * 0.1f;
				const f32 c = std::cos(azimuth);
				const f32 s = std::sin(azimuth);

				const Math::Vec3f eye = target + Math::Vec3f(offset.X() * c + offset.Z() * s, offset.Y(), offset.Z() * c - offset.X() * s);

				Math::Vec2u viewportSize = m_present->GetSceneExtent();
				const Math::Mat4f view = Math::Mat4f::LookAt(eye, target, Math::Vec3f(0.f, 1.f, 0.f));
				const Math::Mat4f proj = Math::Mat4f::PerspectiveRZ(perspective.m_fov, (f32)viewportSize.X() / (f32)viewportSize.Y(),
					perspective.m_nearPlane, perspective.m_farPlane);

				viewProj.m_view = view;
				viewProj.m_projection = proj;
				viewProj.m_viewProjection = proj * view;
				viewProj.m_worldPosition = eye;
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