#include "CreateFolderItem.h"

#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	namespace
	{
		b8 EqualsNoCase(const std::string& lhs, const std::string& rhs)
		{
			if (lhs.size() != rhs.size())
				return false;

			for (usize i = 0; i < lhs.size(); i++)
			{
				if (std::tolower(static_cast<u8>(lhs[i])) != std::tolower(static_cast<u8>(rhs[i])))
					return false;
			}

			return true;
		}

		b8 HasFolderNamed(const DomainFolder* pParent, const std::string& name)
		{
			for (const DomainFolder* pFolder : pParent->GetFolders())
			{
				if (EqualsNoCase(pFolder->GetName(), name))
					return true;
			}

			return false;
		}
	}

	void CreateFolderItem::OnExecute(AssetBrowserContext& context)
	{
		std::string newFolderName = "NewFolder";

		if (HasFolderNamed(context.currentFolder, newFolderName))
		{
			i32 index = 1;
			std::string candidate;

			do
			{
				candidate = std::format("NewFolder({})", index);
				index++;
			} while (HasFolderNamed(context.currentFolder, candidate));

			newFolderName = candidate;
		}

		const std::string newFolderPath = context.currentFolder->GetAbsolutePath() + "/" + newFolderName;

		if (!PAL::Directory::Create(newFolderPath))
			Terminal::Error("NewFolderMenuItem", "Cannot create folder: {}", newFolderPath);
	}

	b8 CreateFolderItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() == 0 && context.selectedFolders.GetCount() == 0;
	}
}