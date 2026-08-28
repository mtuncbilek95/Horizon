#include "DeleteEntityItem.h"

namespace Horizon::Editor
{
	void DeleteEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		if (!context.pCurrWorld)
		{
			Terminal::Error(StringOps::GetName(this), "Somehow current world is nullptr.");
			return;
		}

		for(const auto& entt : context.selectedEntities)
			context.pCurrWorld->DestroyEntity(entt);
	}

	b8 DeleteEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrWorld && !context.selectedEntities.IsEmpty();
	}
}