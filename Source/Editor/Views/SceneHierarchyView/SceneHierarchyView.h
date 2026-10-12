#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/SceneHierarchyMenu/SceneHierarchyContext.h>
#include <Runtime/Containers/List.h>

#include <imgui.h>

#include <map>
#include <set>

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

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

		b8 IsFullBleed() const final { return true; }

	private:
		struct FolderNode
		{
			std::set<std::string> folders;
			List<Engine::EntityHandle> entities;
		};

		struct HierarchyRow
		{
			std::string folder;
			Engine::EntityHandle entity;
			u32 depth = 0;
		};

		struct PendingMove
		{
			std::string folder;
			Engine::EntityHandle dragged;
			Engine::EntityHandle anchor;
			b8 after = false;
			b8 isValid = false;
		};

		void ResetSceneState();
		void CollectEntities();
		void AttachMissingFolders();
		void BuildTree();
		void EnsureFolder(const std::string& path);
		void BuildRows(const std::string& folder, u32 depth);
		b8 IsBefore(Engine::EntityHandle left, Engine::EntityHandle right) const;

		void RenderFolderRow(const HierarchyRow& row);
		void RenderEntityRow(const HierarchyRow& row, usize selectionIndex);
		void RenderVoidTarget();
		void QueueMove(const ImGuiPayload* pPayload, const std::string& folder, Engine::EntityHandle anchor, b8 after);
		void ApplyMove(const List<Engine::EntityHandle>& selected);

		void CreateFolder(const List<Engine::EntityHandle>& selected);
		void RenameFolder(const std::string& path, const std::string& leaf);

		void BeginRename(Engine::EntityHandle handl);
		void BeginFolderRename(const std::string& path);
		void RenderRenameModal();
		void SaveSceneToSource();

	private:
		ContextMenuRegistry<SceneHierarchyContext> m_context;

		ImGuiSelectionBasicStorage m_selection;
		List<Engine::EntityHandle> m_entities;
		List<Engine::EntityHandle> m_sceneEntities;
		List<HierarchyRow> m_rows;

		std::map<std::string, FolderNode> m_tree;
		std::set<std::string> m_sessionFolders;
		std::string m_openFolder;
		PendingMove m_pendingMove;

		std::string m_renamePath;
		std::string m_renameFolder;
		Engine::EntityHandle m_renameHandl;
		c8 m_renameBuffer[256] = {};

		Engine::Scene* m_currentScene = nullptr;
	};
}