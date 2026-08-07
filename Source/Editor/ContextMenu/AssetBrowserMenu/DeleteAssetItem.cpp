#include "DeleteAssetItem.h"

#include <Editor/Domain/DomainNode.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/Log/Terminal.h>

#include <filesystem>

namespace Horizon
{
	void DeleteAssetItem::OnExecute(AssetBrowserMenuContext& context)
	{
		if (context.selected.IsEmpty())
		{
			Terminal::Warn("DeleteAssetItem", "No asset selected");
			return;
		}

		for (DomainNode* node : context.selected)
		{
			if (node->IsFolder())
			{
				std::filesystem::remove_all(node->GetSourcePath());
				Terminal::Info("DeleteAssetItem", "Deleted Folder {}", node->GetName());
				continue;
			}

			PAL::File::Delete(node->GetSourcePath());

			if (!node->GetMetaPath().empty())
				PAL::File::Delete(node->GetMetaPath());

			Terminal::Info("DeleteAssetItem", "Deleted {}", node->GetName());
		}
	}

	b8 DeleteAssetItem::IsEnabled(const AssetBrowserMenuContext& context) const
	{
		return !context.selected.IsEmpty();
	}
}