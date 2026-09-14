#include "RenameEntityItem.h"

namespace Horizon::Editor
{
	void RenameEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		context.renameEntity = context.selectedEntities[0];
	}

	b8 RenameEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene && context.selectedEntities.GetCount() == 1;
	}
}