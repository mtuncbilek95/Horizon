#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["SceneHierarchyView", "Create/Objects/Cube", 2]);
	class H_EXPORT CreateCubeItem : public SceneHierarchyMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateCubeItem);
	public:
		void OnExecute(SceneHierarchyContext& context) final;
		b8 IsEnabled(const SceneHierarchyContext& context) final;
	};
}