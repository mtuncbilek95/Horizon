#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Delete", 2]);
	class H_EXPORT DeleteEntityItem : public SceneHierarchyMenuItem
	{
	public:
		void OnExecute(const SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}