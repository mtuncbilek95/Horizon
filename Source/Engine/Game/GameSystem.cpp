#include "GameSystem.h"

namespace Horizon
{
    GameSystem::GameSystem()
    {
    }

    EngineReport GameSystem::OnInitialize()
    {
        m_gameState = GameState::Start;
        return EngineReport();
    }

    void GameSystem::OnSync()
    {
    }

    void GameSystem::OnFinalize()
    {
    }
}