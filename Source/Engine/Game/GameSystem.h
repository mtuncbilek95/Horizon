#pragma once

#include <Engine/System/System.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Engine/Game/GameState.h>
#include <Engine/ECS/IECS.h>

#include <memory>
#include <vector>

namespace Horizon
{
	class GameSystem : public System<GameSystem>
	{
	public:
		GameSystem() = default;
		virtual ~GameSystem() = default;

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

		GameState GetState() const { return m_state; }

	private:
		GameState m_state = GameState::Idle;
	};
}
