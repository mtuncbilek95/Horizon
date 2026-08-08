#include "CreateEntityItem.h"

namespace Horizon::Editor
{
	void CreateEntityItem::OnExecute(const SceneHierarchyContext& context)
	{
	}

	b8 CreateEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return true;
	}
}