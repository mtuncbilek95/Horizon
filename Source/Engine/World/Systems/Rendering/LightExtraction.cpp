#include "LightExtraction.h"

#include <Engine/Rendering/RenderableAreaLight.h>
#include <Engine/Rendering/RenderablePointLight.h>
#include <Engine/Rendering/RenderableSpotLight.h>
#include <Engine/World/Components/Benchmark/PointLightBatch.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Rendering/AreaLight.h>
#include <Engine/World/Components/Rendering/PointLight.h>
#include <Engine/World/Components/Rendering/SpotLight.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::World::System
{
	b8 LightExtraction::OnInitialize()
	{
		return true;
	}

	void LightExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		auto* pWorld = GetEngine()->RequestService<Engine::WorldService>();
		Engine::Blackboard& board = pWorld->GetBlackboard();

		Engine::FramePointLight& point = board.Add<Engine::FramePointLight>();
		Engine::FrameSpotLight& spot = board.Add<Engine::FrameSpotLight>();
		Engine::FrameAreaLight& area = board.Add<Engine::FrameAreaLight>();

		currentScene.ForEach<Component::LocalTransform, Component::PointLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::PointLight& light)
			{
				point.pointLights.PushBack({ light.m_color, local.m_position, light.m_intensity, light.m_range });
			});

		currentScene.ForEach<Component::PointLightBatch>([&](Engine::EntityHandle, Component::PointLightBatch& batch)
			{
				for (const Engine::RenderablePointLight& light : batch.m_lights)
					point.pointLights.PushBack(light);
			});

		currentScene.ForEach<Component::LocalTransform, Component::SpotLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::SpotLight& light)
			{
				const Math::Quat rotation = Math::Quat::FromEuler(local.m_rotation);

				spot.spotLights.PushBack({ light.m_color, local.m_position,
					rotation.RotateVector(Math::Vec3f(0.f, 0.f, -1.f)),
					light.m_intensity, light.m_range, light.m_innerConeAngle, light.m_outerConeAngle });
			});

		currentScene.ForEach<Component::LocalTransform, Component::AreaLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::AreaLight& light)
			{
				const Math::Quat rotation = Math::Quat::FromEuler(local.m_rotation);

				area.areaLights.PushBack({ light.m_color, local.m_position,
					rotation.RotateVector(Math::Vec3f(1.f, 0.f, 0.f)),
					rotation.RotateVector(Math::Vec3f(0.f, 1.f, 0.f)),
					light.m_intensity, light.m_width, light.m_height, light.m_twoSided });
			});
	}

	void LightExtraction::OnFinalize()
	{
	}

	void LightExtraction::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::LocalTransform>();
		graph.Reads<Component::PointLight>();
		graph.Reads<Component::PointLightBatch>();
		graph.Reads<Component::SpotLight>();
		graph.Reads<Component::AreaLight>();

		graph.Writes<Engine::FramePointLight>();
		graph.Writes<Engine::FrameSpotLight>();
		graph.Writes<Engine::FrameAreaLight>();
	}
}