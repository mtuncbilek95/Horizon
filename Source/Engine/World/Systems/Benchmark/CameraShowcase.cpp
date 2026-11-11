#include "CameraShowcase.h"

#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Benchmark/CameraRoller.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>
#include <Runtime/Math/Scalar.h>

#include <cmath>

namespace Horizon::World::System
{
	b8 CameraShowcase::OnInitialize()
	{
		return true;
	}

	void CameraShowcase::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		currentScene.ForEach<Component::LocalTransform, Component::CameraRoller>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::CameraRoller& roller)
			{
				roller.m_azimuth += roller.m_rotationSpeed * ctx.DeltaTime();

				const f32 cosElevation = std::cos(roller.m_elevation);
				const f32 sinElevation = std::sin(roller.m_elevation);
				const f32 cosAzimuth = std::cos(roller.m_azimuth);
				const f32 sinAzimuth = std::sin(roller.m_azimuth);

				local.m_position = Math::Vec3f(cosElevation * sinAzimuth, sinElevation, cosElevation * cosAzimuth) * roller.m_distance;
				local.m_rotation = Math::Vec3f(-roller.m_elevation, roller.m_azimuth, 0.f);
			});
	}

	void CameraShowcase::OnFinalize()
	{
	}

	void CameraShowcase::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Writes<Component::CameraRoller>();
		graph.Writes<Component::LocalTransform>();

		graph.RunsBefore<LocalToWorld>();
	}
}