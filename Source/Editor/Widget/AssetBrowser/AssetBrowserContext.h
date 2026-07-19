#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	struct AssetBrowserContext
	{
		DomainFolder* currentFolder;
		std::vector<DomainFile*> selectedFiles;
	};
}