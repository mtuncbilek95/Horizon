#include "ECSSystem.h"

#include <Engine/ECS/Mesh/MeshComponentSystem.h>
#include <Engine/ECS/Camera/CameraComponentSystem.h>
#include <Engine/ECS/Transform/TransformComponentSystem.h>

#include <Engine/Game/GameSystem.h>

namespace Horizon
{
	SystemReport ECSSystem::OnInitialize()
	{
		m_ecsSystems.clear();

		// Initialize all ECS systems.
		auto mesh = std::make_shared<MeshComponentSystem>();
		mesh->SetEngine(GetEngine());
		mesh->SetScene(nullptr);
		m_ecsSystems.push_back(mesh);

		auto camera = std::make_shared<CameraComponentSystem>();
		camera->SetEngine(GetEngine());
		camera->SetScene(nullptr);
		m_ecsSystems.push_back(camera);

		auto transform = std::make_shared<TransformComponentSystem>();
		transform->SetEngine(GetEngine());
		transform->SetScene(nullptr);
		m_ecsSystems.push_back(transform);

		return SystemReport();
	}

	void ECSSystem::OnSync()
	{
		auto& gameSystem = RequestSystem<GameSystem>();
		auto currentState = gameSystem.GetState();

		if(currentState == GameState::Starting)
		{
			for (auto& ecs : m_ecsSystems)
			{
				if (!(*ecs).OnStart())
					return;
			}
		}
		else if(currentState == GameState::Running)
		{
			for (auto& ecs : m_ecsSystems)
				ecs->OnTick();
		}
		else if(currentState == GameState::Paused)
		{
			for (auto& ecs : m_ecsSystems)
				ecs->OnStop();
		}
	}

	void ECSSystem::OnFinalize()
	{
	}
}