#include "CreateFolderItem.h"

#include <Editor/Domain/DomainNode.h>
#include <regex>

namespace Horizon
{
	void CreateFolderItem::OnExecute(AssetBrowserMenuContext& context)
	{
		if (!context.selected.IsEmpty())
			return;

		const auto& path = context.currentNode->GetSourcePath();

		static const std::regex desen(R"(NewFolder(?:\((\d+)\))?)", std::regex::icase);

		int maxNum = -1;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
				continue;

			const std::string name = entry.path().filename().string();
			std::smatch m;
			if (std::regex_match(name, m, desen))
			{
				const int num = m[1].matched ? std::stoi(m[1].str()) : 0;
				maxNum = std::max(maxNum, num);
			}
		}

		const std::string folderName = (maxNum < 0)
			? "NewFolder" : "NewFolder(" + std::to_string(maxNum + 1) + ")";

		if (!std::filesystem::create_directory(path / folderName))
			Terminal::Error("CreateFolder", "Folder could not be created!");
	}

	b8 CreateFolderItem::IsEnabled(const AssetBrowserMenuContext& context) const
	{
		return context.selected.GetCount() == 0;
	}
}