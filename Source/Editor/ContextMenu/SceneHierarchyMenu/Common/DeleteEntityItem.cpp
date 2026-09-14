#include "DeleteEntityItem.h"

namespace Horizon::Editor
{
	void DeleteEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		for (usize i = 0; i < context.selectedEntities.GetCount(); i++)
			context.pCurrentScene->RemoveEntity(context.selectedEntities[i]);
	}

	b8 DeleteEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene && context.selectedEntities.GetCount() > 0;
	}
}