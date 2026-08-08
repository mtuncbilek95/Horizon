#include "CreateFolderItem.h"

namespace Horizon::Editor
{
	void CreateFolderItem::OnExecute(const AssetBrowserContext& context)
	{
	}

	b8 CreateFolderItem::IsEnabled(const AssetBrowserContext& context)
	{
		return true;
	}
}