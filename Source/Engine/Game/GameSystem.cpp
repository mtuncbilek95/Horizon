#include "GameSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>

#include <Engine/Graphics/GraphicsSystem.h>

namespace Horizon
{
	SystemReport GameSystem::OnInitialize()
	{
		m_state = GameState::Starting;
		return SystemReport();
	}

	void GameSystem::OnSync()
	{
		if (m_state == GameState::Starting)
		{
			m_state = GameState::Running;
		}
	}

	void GameSystem::OnFinalize()
	{
	}
}
