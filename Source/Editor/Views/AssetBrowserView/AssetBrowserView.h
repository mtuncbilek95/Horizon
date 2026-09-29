#pragma once

#include <Editor/AssetAction/AssetAction.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserContext.h>
#include <Runtime/Containers/List.h>

#include <imgui.h>
#include <string>
#include <unordered_map>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_FOLDER_TREE, "Asset Browser", false, true, DockZone::Bottom]);
	class H_EXPORT AssetBrowserView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(AssetBrowserView);

		struct BrowserEntry
		{
			DomainFolder* pFolder = nullptr;
			DomainFile* pFile = nullptr;
			u32 id;

			b8 IsFolder() const { return pFolder; }

			const std::string& GetPath() const;
			const std::string& GetName() const;
		};

	public:
		AssetBrowserView() = default;
		~AssetBrowserView() final;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

	private:
		void RenderToolbar(DomainFolder* pFolder);
		void RebuildEntries(DomainFolder* pFolder);
		void RenderGrid();
		void DrawCell(ImDrawList* pDrawList, const BrowserEntry& entry, const ImVec2& cellMin, f32 cell);
		void CollectSelected(AssetBrowserContext& context);
		void Navigate(DomainFolder* pTarget);
		void BeginRename(const std::string& sourcePath);
		void RenderRenameModal();
		BrowserEntry* FindEntry(const std::string& path);

	private:
		// Asset Open Actions
		List<AssetAction*> m_openActions;
		std::unordered_map<std::string, usize> m_actionNameLookup;

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