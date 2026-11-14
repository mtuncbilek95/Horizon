#include "InputActionMap.h"

#include <Engine/Asset/Input/InputActionMapLoader.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Asset
{
	InputActionMap::InputActionMap()
	{
	}

	InputActionMap::~InputActionMap()
	{
	}

	void InputActionMap::LoadSync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<InputActionMapLoader>()->LoadSync(this);
	}

	void InputActionMap::UnloadSync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Resident, Engine::AssetResidency::Pending) != Engine::AssetResidency::Resident)
			return;

		GetLoader<InputActionMapLoader>()->UnloadSync(this);
	}

	i64 InputActionMap::FindActionIndex(std::string_view name) const
	{
		for (usize i = 0; i < m_actions.GetCount(); ++i)
		{
			if (m_actions[i].name == name)
				return static_cast<i64>(i);
		}

		Terminal::Error("InputActionMap", "{} has no action named {}", m_id.ToString(), name);
		return -1;
	}
}
