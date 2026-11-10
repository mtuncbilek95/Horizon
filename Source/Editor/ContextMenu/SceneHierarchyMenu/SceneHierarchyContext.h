#pragma once

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Vector/List.h>
#include <Engine/World/ECS/Scene.h>

namespace Horizon::Editor
{
	struct SceneHierarchyContext
	{
		Engine::Engine* pEngine = nullptr;
		Engine::Scene* pCurrentScene = nullptr;

		List<Engine::EntityHandle> selectedEntities;

		Engine::EntityHandle renameEntity;
	};
}