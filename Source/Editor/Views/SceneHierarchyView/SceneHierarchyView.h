#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyContext.h>
#include <Runtime/Containers/List.h>

#include <imgui.h>

namespace Horizon::Engine
{
	class World;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_DIAGRAM_PROJECT, "Scene Hierarchy", false, true, DockZone::Left]);
	class H_EXPORT SceneHierarchyView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneHierarchyView);
	public:
		void OnInvoke() final;
		void OnRender() final;
	};
}