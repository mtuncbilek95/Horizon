#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Delete Folder", i32_max]);
	class EDITOR_API DeleteEntityFolderItem : public SceneHierarchyMenuItem
	{
		HORIZON_TYPE_REFLECT(DeleteEntityFolderItem);
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}