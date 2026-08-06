#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuContext.h>
#include <Editor/Domain/DomainNode.h>

#include <imgui.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	class DomainSystem;

	HCLASS(EditorView[ICON_FA_FOLDER_TREE, "Asset Browser", false, true, DockZone::Bottom]);
	class H_EXPORT AssetBrowserView : public ViewObject
	{
	public:
		void OnInvoke() final;
		void OnRender() final;

	private:
		void CollectSelected(List<DomainNode*>& out) const;

		static DomainNode* FindFolder(DomainNode* node, const std::filesystem::path& path);
		static ImGuiID PathToId(const std::filesystem::path& path);
		static ImGuiID AdapterIndexToStorageId(ImGuiSelectionBasicStorage* self, i32 index);

		void RenameOnDisk(const std::filesystem::path& sourcePath, const char* newName);
	private:
		DomainSystem* m_domain = nullptr;
		std::filesystem::path m_currentPath;

		List<DomainNode*> m_children;
		ImGuiSelectionBasicStorage m_selection;
		ContextMenuRegistry<AssetBrowserMenuContext> m_contextMenu;

		std::filesystem::path m_renamePath;
		char m_renameBuffer[256] = {};
		b8 m_renameOpen = false;
	};
}