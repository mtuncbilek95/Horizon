#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyContext.h>
#include <Runtime/Containers/List.h>

#include <imgui.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_DIAGRAM_PROJECT, "Scene Hierarchy", DockZone::Left, EditorViewFlags::OpenOnStart]);
	class EDITOR_API SceneHierarchyView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneHierarchyView);
	public:
		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
		b8 OnCommand(ViewCommand command) final;

		b8 IsFullBleed() const final { return true; }

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final { m_context.OnLibraryRegistered(library); }
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final { m_context.OnLibraryUnregistered(library); }

	private:
		void BeginRename(Engine::EntityHandle handl);
		void RenderRenameModal();
		void SaveSceneToSource();

	private:
		ContextMenuRegistry<SceneHierarchyContext> m_context;

		ImGuiSelectionBasicStorage m_selection;
		List<Engine::EntityHandle> m_entities;

		std::string m_renamePath;
		Engine::EntityHandle m_renameHandl;
		c8 m_renameBuffer[256] = {};

		Engine::Scene* m_currentScene = nullptr;
	};
}