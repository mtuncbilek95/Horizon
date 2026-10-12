#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Create/Folder", 1]);
	class EDITOR_API CreateEntityFolderItem : public SceneHierarchyMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateEntityFolderItem);
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}