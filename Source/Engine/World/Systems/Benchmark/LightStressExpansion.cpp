#include "LightStressExpansion.h"

#include <Engine/World/SystemDependency.h>
#include <Engine/World/Components/Benchmark/PointLightBatch.h>
#include <Engine/World/Components/Benchmark/LightStressTest.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Rendering/PointLight.h>
#include <Engine/World/Systems/Rendering/RenderSubmission.h>

#include <cmath>

namespace Horizon::World::System
{
	namespace
	{
		f32 Random01(u32 seed)
		{
			const u32 state = seed * 747796405u + 2891336453u;
			const u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;

			return static_cast<f32>((word >> 22u) ^ word) / 4294967295.0f;
		}
	}

	b8 LightStressExpansion::OnInitialize()
	{
		return true;
	}

	void LightStressExpansion::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		const f32 time = (f32)ctx.GetTotalSeconds();
		const f32 twoPi = 6.2831853f;

		currentScene.ForEach<Component::LocalTransform, Component::PointLight, Component::LightStressTest, Component::PointLightBatch>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::PointLight& light, Component::LightStressTest& stress, Component::PointLightBatch& batch)
			{
				batch.m_lights.Clear();
				batch.m_lights.Reserve(stress.m_lightInstanceCount);

				for (u32 i = 0; i < stress.m_lightInstanceCount; i++)
				{
					const u32 seed = i * 8u;

					const f32 ring = stress.m_innerRadius + (stress.m_outerRadius - stress.m_innerRadius) * std::sqrt(Random01(seed));
					const f32 startAngle = Random01(seed + 1u) * twoPi;
					const f32 direction = Random01(seed + 2u) < 0.5f ? -1.f : 1.f;
					const f32 speed = direction * (stress.m_minSpeed + (stress.m_maxSpeed - stress.m_minSpeed) * Random01(seed + 3u));
					const f32 height = stress.m_minHeight + (stress.m_maxHeight - stress.m_minHeight) * Random01(seed + 4u);
					const f32 bobPhase = Random01(seed + 5u) * twoPi;
					const f32 hue = Random01(seed + 6u) * twoPi;

					const f32 angle = startAngle + time * speed;
					const f32 bob = stress.m_bobAmplitude * std::sin(time * (0.5f + std::fabs(speed)) + bobPhase);

					const Math::Vec3f offset(std::cos(angle) * ring, height + bob, std::sin(angle) * ring);
					const Math::Color4f color(0.5f + 0.5f * std::cos(hue), 0.5f + 0.5f * std::cos(hue + 2.094f), 0.5f + 0.5f * std::cos(hue + 4.188f), 1.f);

					batch.m_lights.PushBack({ color, local.m_position + offset, light.m_intensity, light.m_range });
				}
			});
	}

	void LightStressExpansion::OnFinalize()
	{
	}

	void LightStressExpansion::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::LocalTransform>();
		graph.Reads<Component::PointLight>();
		graph.Reads<Component::LightStressTest>();
		graph.Writes<Component::PointLightBatch>();

		graph.RunsBefore<RenderSubmission>();
	}
}