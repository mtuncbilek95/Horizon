#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Core/Application.h>

namespace Horizon::Editor
{
	struct H_EXPORT AssetBrowserContext
	{
		Engine::Application* pEngine;
		DomainFolder* currentFolder = nullptr;

		List<DomainFolder*> selectedFolders;
		List<DomainFile*> selectedFiles;

		std::string renamePath;
	};
}