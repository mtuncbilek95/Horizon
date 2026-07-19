#include "CreateFolderAssetBrowserMenu.h"

#include <Runtime/Log/Terminal.h>

#include <regex>
#include <algorithm>
#include <string>

namespace Horizon
{
	void CreateFolderAssetBrowserMenu::OnInvoke(const AssetBrowserContext& context)
	{
		if (!context.currentFolder)
		{
			Terminal::Warn("CreateFolderAssetBrowserMenu", "target folder is a valid folder in vfs");
			return;
		}

		// Some magic regex stuff (WTF?)
		const std::regex reg(R"(^New\s*Folder(?:\s*\((\d+)\))?$)", std::regex::icase);

		i64 nameCounter = -1;
		for (const auto& entry : std::filesystem::directory_iterator(context.currentFolder->GetAbsolutePath()))
		{
			if (!entry.is_directory())
				continue;

			std::smatch newMatch;

			const std::string name = entry.path().filename().string();
			if (std::regex_match(name, newMatch, reg))
			{
				i64 n = newMatch[1].matched ? std::stoi(newMatch[1].str()) : 0;
				nameCounter = std::max(nameCounter, n);
			}
		}

		std::filesystem::path newPath = context.currentFolder->GetAbsolutePath();
		if (nameCounter < 0)
			newPath /= "New Folder";
		else
			newPath /= "New Folder (" + std::to_string(nameCounter + 1) + ")";

		std::filesystem::create_directory(newPath);
	}
}