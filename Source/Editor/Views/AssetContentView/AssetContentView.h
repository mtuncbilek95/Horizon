#pragma once

#include <Editor/AssetAction/AssetAction.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserContext.h>
#include <Editor/Components/PropertyRenderer.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Runtime/Containers/List.h>

#include <imgui.h>
#include <string>
#include <unordered_map>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_FOLDER_TREE, "Content Browser", DockZone::Bottom, EditorViewFlags::OpenOnStart]);
	class EDITOR_API AssetContentView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(AssetContentView);

		struct BrowserEntry
		{
			DomainFolder* pFolder = nullptr;
			DomainFile* pFile = nullptr;
			u32 id;

			b8 IsFolder() const { return pFolder; }

			const std::string& GetPath() const;
			const std::string& GetName() const;
		};

		struct ActionEntry
		{
			const Reflect::Type* pType = nullptr;
			AssetAction* pAction = nullptr;
		};

	public:
		AssetContentView() = default;
		~AssetContentView() final;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
		b8 AddAction(const Reflect::Type* pType);
		void RemoveAction(const Reflect::Type* pType);

		void RenderToolbar(DomainFolder* pFolder);
		void RebuildEntries(DomainFolder* pFolder);
		void RenderGrid();
		void DrawCell(ImDrawList* pDrawList, const BrowserEntry& entry, const ImVec2& cellMin, f32 cell);
		void CollectSelected(AssetBrowserContext& context);
		void Navigate(DomainFolder* pTarget);
		void BeginRename(const std::string& sourcePath);
		void RenderRenameModal();
		BrowserEntry* FindEntry(const std::string& path);

		// Import action
		void ImportFiles(const List<std::string>& files);
		void RenderDialog();
	private:
		// Asset Open Actions
		List<ActionEntry> m_openActions;
		std::unordered_map<std::string, AssetAction*> m_actionNameLookup;

		// Context Menu
		ContextMenuRegistry<AssetBrowserContext> m_contextMenu;
		DomainFolder* m_currentFolder = nullptr;

		// Safe-use entries
		List<BrowserEntry> m_entries;
		ImGuiSelectionBasicStorage m_selection;

		// Rename misc.
		std::string m_renamePath;
		b8 m_renameIsFolder = false;
		c8 m_renameBuffer[256] = {};
	};
}