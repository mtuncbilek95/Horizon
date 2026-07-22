#include "CreateSceneAssetBrowserMenu.h"

#include <Editor/Domain/DomainSystem.h>
#include <Editor/Domain/ImportPipeline/ImportDescriptor.h>
#include <Engine/Asset/World/WorldAsset.h>
#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>
#include <Runtime/Log/Terminal.h>

#include <regex>
#include <algorithm>
#include <string>

namespace Horizon
{
	void CreateSceneAssetBrowserMenu::OnInvoke(const AssetBrowserContext& context)
	{
		if (!context.currentFolder)
		{
			Terminal::Warn("CreateSceneAssetBrowserMenu", "target folder is not valid in vfs");
			return;
		}

		const std::regex reg(R"(^New\s*Scene(?:\s*\((\d+)\))?$)", std::regex::icase);

		i64 nameCounter = -1;
		for (const auto& entry : std::filesystem::directory_iterator(context.currentFolder->GetFolderPath()))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".hmeta")
				continue;

			std::smatch match;

			const std::string name = entry.path().stem().string();
			if (std::regex_match(name, match, reg))
			{
				i64 n = match[1].matched ? std::stoi(match[1].str()) : 0;
				nameCounter = std::max(nameCounter, n);
			}
		}

		std::string sceneName = (nameCounter < 0) ? "New Scene" : "New Scene (" + std::to_string(nameCounter + 1) + ")";

		auto& domainSub = GetEngine()->GetSystem<DomainSystem>();
		auto* moduleCtx = GetEngine()->GetModuleContext();

		Reflect::Type* worldType = moduleCtx->GetType(Reflect::TypeOf<WorldAsset>());

		ImportDescriptor newSceneInfo = {};
		newSceneInfo.fileName = sceneName;
		newSceneInfo.fileType = worldType;

		domainSub.ImportDefault(context.currentFolder, newSceneInfo);
	}
}