#include "RenameObjectItem.h"

namespace Horizon::Editor
{
	void RenameObjectItem::OnExecute(AssetBrowserContext& context)
	{
		if (context.selectedFiles.GetCount() == 1 && context.selectedFolders.GetCount() == 0)
			context.renamePath = context.selectedFiles[0]->GetSourcePath();
		else if (context.selectedFolders.GetCount() == 1 && context.selectedFiles.GetCount() == 0)
			context.renamePath = context.selectedFolders[0]->GetAbsolutePath();
	}

	b8 RenameObjectItem::IsEnabled(const AssetBrowserContext& context)
	{
		bool oneFileSelected = context.selectedFiles.GetCount() == 1;
		bool oneFolderSelected = context.selectedFolders.GetCount() == 1;
		return oneFileSelected != oneFolderSelected;
	}
}