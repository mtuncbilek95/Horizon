#pragma once

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/List.h>
#include <Engine/World/ECS/Scene.h>

namespace Horizon::Editor
{
	struct EDITOR_API SceneHierarchyContext
	{
		Engine::Engine* pEngine = nullptr;
		Engine::Scene* pCurrentScene = nullptr;

		List<Engine::EntityHandle> selectedEntities;
		List<std::string> selectedFolders;

		Engine::EntityHandle renameEntity;
		Engine::EntityHandle newIcon;
		b8 createFolder = false;
		b8 deleteFolder = false;
	};
}