#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Rename", 1]);
	class H_EXPORT RenameEntityItem : public SceneHierarchyMenuItem
	{
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}