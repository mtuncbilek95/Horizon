#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Create/Folder", 1]);
	class EDITOR_API CreateFolderItem : public SceneHierarchyMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateFolderItem);
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}