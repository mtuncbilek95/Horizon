#include "DeleteEntityItem.h"

namespace Horizon::Editor
{
	void DeleteEntityItem::OnExecute(SceneHierarchyContext& context)
	{
	}

	b8 DeleteEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return true;
	}
}