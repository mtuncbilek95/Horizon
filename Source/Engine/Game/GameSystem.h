#pragma once

#include <Engine/System/System.h>

namespace Horizon
{
    enum class GameState
    {
        Idle,
        Start,
        Run,
        Pause,
        Stop
    };

    class GameSystem final : public System<GameSystem>
    {
    public:
        GameSystem();
        ~GameSystem() = default;

        GameState GetState() const { return m_gameState; }
        void SetState(GameState state) { m_gameState = state; }

    private:
        EngineReport OnInitialize();
        void OnSync();
        void OnFinalize();

    private:
        GameState m_gameState = GameState::Idle;
    };
}