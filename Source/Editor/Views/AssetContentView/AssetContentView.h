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
#include <Runtime/Containers/Memory/RefPtr.h>
#include <Runtime/Containers/Vector/List.h>

#include <imgui.h>
#include <string>
#include <unordered_map>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_FOLDER_TREE, "Content Browser", DockZone::Bottom, EditorViewFlags::OpenOnStart]);
	class AssetContentView : public ViewObject
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
			RefPtr<AssetAction> pAction;
		};

	public:
		AssetContentView() = default;
		~AssetContentView() final = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

	private:
		b8 AddAction(const Reflect::Type* pType);
		void RemoveAction(const Reflect::Type* pType);

		void RenderToolbar(DomainFolder* pFolder);
		void RebuildEntries(DomainFolder* pFolder);
		void CollectSelected(AssetBrowserContext& context);
		void Navigate(DomainFolder* pTarget);
		void BeginRename(const std::string& sourcePath);
		void RenderRenameModal();
		BrowserEntry* FindEntry(const std::string& path);

		void ImportFiles(const List<std::string>& files);
		void RenderDialog();

	private:
		List<ActionEntry> m_openActions;
		std::unordered_map<std::string, RefPtr<AssetAction>> m_actionNameLookup;

		ContextMenuRegistry<AssetBrowserContext> m_contextMenu;
		DomainFolder* m_currentFolder = nullptr;

		List<BrowserEntry> m_entries;
		ImGuiSelectionBasicStorage m_selection;

		std::string m_renamePath;
		b8 m_renameIsFolder = false;
		c8 m_renameBuffer[256] = {};
	};
}