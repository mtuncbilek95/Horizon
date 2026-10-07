#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Core/Engine.h>

namespace Horizon::Editor
{
	struct EDITOR_API AssetBrowserContext
	{
		Engine::Engine* pEngine;
		DomainFolder* pCurrentFolder = nullptr;

		List<DomainFolder*> selectedFolders;
		List<DomainFile*> selectedFiles;

		std::string renamePath;
	};
}