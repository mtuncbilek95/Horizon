#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyContext.h>
#include <Engine/World/Definitions.h>
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

		struct HierarchyRow
		{
			Engine::EntityHandle entity;
			ImGuiID id = 0;
		};

	public:
		void OnInvoke() final;
		void OnRender() final;

		void SetCurrentWorld(Engine::World* pCurrentWorld);

	private:
		void RebuildRows();
		void RenderRows();
		void SyncSelectionModel();
		void CollectSelected(SceneHierarchyContext& context);

	private:
		ContextMenuRegistry<SceneHierarchyContext> m_contextMenu;

		Engine::World* m_currentWorld = nullptr;

		List<HierarchyRow> m_rows;
		ImGuiSelectionBasicStorage m_multiSelect;
	};
}