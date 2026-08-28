#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/World/World.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	struct H_EXPORT SceneHierarchyContext
	{
		Engine::Engine* pEngine = nullptr;
		Engine::World* pCurrWorld = nullptr;

		List<Engine::EntityHandle> selectedEntities;
		Engine::EntityHandle renameHandl;
	};
}