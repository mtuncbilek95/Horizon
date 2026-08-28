#include "RenameEntityItem.h"

#include <Engine/World/Components/NameComponent.h>

namespace Horizon::Editor
{
	void RenameEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		if (!context.pCurrWorld)
		{
			Terminal::Error(StringOps::GetName(this), "Somehow current world is nullptr.");
			return;
		}

		context.renameHandl = context.selectedEntities[0];
	}

	b8 RenameEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrWorld && context.selectedEntities.GetCount() == 1;
	}
}