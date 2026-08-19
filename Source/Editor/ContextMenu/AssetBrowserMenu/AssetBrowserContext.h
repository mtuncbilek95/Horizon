#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Core/Engine.h>

namespace Horizon::Editor
{
	struct H_EXPORT AssetBrowserContext
	{
		Engine::Engine* pEngine;
		DomainFolder* currentFolder = nullptr;

		List<DomainFolder*> selectedFolders;
		List<DomainFile*> selectedFiles;

		std::string renamePath;
	};
}