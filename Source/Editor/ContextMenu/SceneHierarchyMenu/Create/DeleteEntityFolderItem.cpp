#include "DeleteEntityFolderItem.h"

namespace Horizon::Editor
{
	void DeleteEntityFolderItem::OnExecute(SceneHierarchyContext& context)
	{
		context.deleteFolder = true;
	}

	b8 DeleteEntityFolderItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene && context.selectedFolders.GetCount() > 0;
	}
}