#include "EntityComponentSystem.h"

#include <Engine/EntityComponent/Systems/CameraSystem.h>
#include <Engine/EntityComponent/Systems/MeshRendererSystem.h>

#include <Engine/Game/GameSystem.h>

namespace Horizon
{
    EntityComponentSystem::EntityComponentSystem()
    {
    }

    EngineReport EntityComponentSystem::OnInitialize()
    {
        AddSystem<CameraSystem>();
        AddSystem<MeshRendererSystem>();

        return EngineReport();
    }

    void EntityComponentSystem::OnSync()
    {
        auto& gameSystem = RequestSystem<GameSystem>();

        switch (gameSystem.GetState())
        {
        case GameState::Start:
        {
            for (auto& [index, system] : m_entitySystems)
                system->OnStart();

            gameSystem.SetState(GameState::Run);
            break;
        }
        case GameState::Run:
        {
            for (auto& [index, system] : m_entitySystems)
                system->OnTick();

            break;
        }
        case GameState::Stop:
        {
            for (auto& [index, system] : m_entitySystems)
                system->OnStop();

            gameSystem.SetState(GameState::Idle);
            break;
        }
        case GameState::Idle:
        case GameState::Pause:
            break;
        }
    }

    void EntityComponentSystem::OnFinalize()
    {
    }
}