#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "SetIcon", i32_max]);
	class EDITOR_API SetEntityIconItem : public SceneHierarchyMenuItem
	{
		HORIZON_TYPE_REFLECT(SetEntityIconItem);
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}