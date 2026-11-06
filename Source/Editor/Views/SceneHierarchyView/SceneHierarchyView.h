#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyContext.h>
#include <Runtime/Containers/Vector/List.h>

#include <imgui.h>

#include <map>
#include <set>
#include <string_view>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_DIAGRAM_PROJECT, "Scene Hierarchy", DockZone::Left, EditorViewFlags::OpenOnStart]);
	class SceneHierarchyView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneHierarchyView);
	public:
		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
		b8 OnCommand(ViewCommand command) final;

		b8 IsFullBleed() const final { return true; }
	};
}