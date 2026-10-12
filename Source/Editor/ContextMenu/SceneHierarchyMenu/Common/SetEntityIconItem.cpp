#include "SetEntityIconItem.h"

namespace Horizon::Editor
{
	void SetEntityIconItem::OnExecute(SceneHierarchyContext& context)
	{
		context.newIcon = context.selectedEntities[0];
	}

	b8 SetEntityIconItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene && context.selectedEntities.GetCount() == 1;
	}
}