#include "CreateFolderItem.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	namespace
	{
		b8 HasFolderNamed(const DomainFolder* pParent, const std::string& name)
		{
			for (const DomainFolder* pFolder : pParent->GetFolders())
			{
				if (StringHelper::CheckIfEquals(pFolder->GetName(), name))
					return true;
			}

			return false;
		}
	}

	void CreateFolderItem::OnExecute(AssetBrowserContext& context)
	{
		std::string newFolderName = "NewFolder";

		if (HasFolderNamed(context.pCurrentFolder, newFolderName))
		{
			i32 index = 1;
			std::string candidate;

			do
			{
				candidate = std::format("NewFolder({})", index);
				index++;
			} while (HasFolderNamed(context.pCurrentFolder, candidate));

			newFolderName = candidate;
		}

		const std::string newFolderPath = context.pCurrentFolder->GetAbsolutePath() + "/" + newFolderName;

		if (!PAL::Directory::Create(newFolderPath))
			Terminal::Error(StringHelper::GetTypeName(this), "Cannot create folder: {}", newFolderPath);
	}

	b8 CreateFolderItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() == 0 && context.selectedFolders.GetCount() == 0;
	}
}