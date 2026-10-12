#include "CreateEntityFolderItem.h"

namespace Horizon::Editor
{
	void CreateEntityFolderItem::OnExecute(SceneHierarchyContext& context)
	{
		context.createFolder = true;
	}

	b8 CreateEntityFolderItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene;
	}
}