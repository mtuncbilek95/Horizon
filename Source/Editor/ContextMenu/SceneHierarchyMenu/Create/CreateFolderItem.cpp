#include "CreateFolderItem.h"

namespace Horizon::Editor
{
	void CreateFolderItem::OnExecute(SceneHierarchyContext& context)
	{
		context.createFolder = true;
	}

	b8 CreateFolderItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene;
	}
}