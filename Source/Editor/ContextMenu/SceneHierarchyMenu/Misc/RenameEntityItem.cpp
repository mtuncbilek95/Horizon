#include "RenameEntityItem.h"

namespace Horizon::Editor
{
	void RenameEntityItem::OnExecute(const SceneHierarchyContext& context)
	{
	}

	b8 RenameEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return true;
	}
}