#include "AssetBrowserView.h"

#include <Editor/Domain/DomainSystem.h>

#include <Engine/Core/Engine.h>

namespace fs = std::filesystem;

namespace Horizon
{
	void AssetBrowserView::OnInvoke()
	{
		m_selection.UserData = this;
		m_selection.AdapterIndexToStorageId = &AssetBrowserView::AdapterIndexToStorageId;

		m_contextMenu.Bootstrap(m_engine, "AssetBrowser");
	}

	void AssetBrowserView::OnRender()
	{
		if (!m_domain)
			m_domain = m_engine->TryGetSystem<DomainSystem>();

		if (!m_domain)
		{
			ImGui::TextDisabled("DomainSystem not available");
			return;
		}

		DomainNode* root = m_domain->GetRoot();
		if (!root)
		{
			ImGui::TextDisabled("No assets");
			return;
		}

		DomainNode* folder = m_currentPath.empty() ? root : FindFolder(root, m_currentPath);
		if (!folder)
			folder = root;

		if (folder->GetParent())
		{
			if (ImGui::Button(ICON_FA_ARROW_UP " Up"))
				m_currentPath = folder->GetParent()->GetSourcePath();

			ImGui::SameLine();
		}
		ImGui::TextDisabled("%s", folder->GetSourcePath().string().c_str());
		ImGui::Separator();

		m_children.Clear();
		for (auto* child : folder->GetItemList())
			m_children.PushBack(child);

		const i32 count = static_cast<i32>(m_children.GetCount());

		const ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;
		ImGuiMultiSelectIO* io = ImGui::BeginMultiSelect(flags, m_selection.Size, count);
		m_selection.ApplyRequests(io);

		const f32 cell = ImGui::GetFontSize() * 4.0f;
		const f32 wrapX = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

		fs::path enterFolder;

		for (i32 i = 0; i < count; i++)
		{
			DomainNode* child = m_children[static_cast<usize>(i)];
			const ImGuiID id = PathToId(child->GetSourcePath());
			const b8 selected = m_selection.Contains(id);

			const char* icon = child->IsFolder() ? ICON_FA_FOLDER : ICON_FA_FILE;
			const std::string label = std::string(icon) + " " + child->GetName();

			ImGui::PushID(i);
			ImGui::SetNextItemSelectionUserData(i);
			ImGui::Selectable(label.c_str(), selected, 0, ImVec2(cell, cell));

			if (child->IsFolder() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				enterFolder = child->GetSourcePath();

			ImGui::PopID();

			if (ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + cell < wrapX)
				ImGui::SameLine();
		}

		io = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(io);

		List<DomainNode*> selected;
		CollectSelected(selected);

		AssetBrowserMenuContext context{ m_engine, selected, &m_renamePath };
		m_contextMenu.RenderPopup("asset_browser_ctx", context);

		if (!m_renamePath.empty() && !m_renameOpen)
		{
			const std::string stem = m_renamePath.stem().string();
			std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", stem.c_str());

			ImGui::OpenPopup("Rename Asset");
			m_renameOpen = true;
		}

		if (ImGui::BeginPopupModal("Rename Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer));

			if (ImGui::Button("OK"))
			{
				RenameOnDisk(m_renamePath, m_renameBuffer);
				m_renamePath.clear();
				m_renameOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				m_renamePath.clear();
				m_renameOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (!enterFolder.empty())
			m_currentPath = enterFolder;
	}

	void AssetBrowserView::CollectSelected(List<DomainNode*>& out) const
	{
		for (auto* child : m_children)
		{
			if (m_selection.Contains(PathToId(child->GetSourcePath())))
				out.PushBack(child);
		}
	}

	DomainNode* AssetBrowserView::FindFolder(DomainNode* node, const fs::path& path)
	{
		if (!node)
			return nullptr;

		if (node->GetSourcePath() == path)
			return node;

		for (auto* child : node->GetItemList())
		{
			if (!child->IsFolder())
				continue;

			if (auto* found = FindFolder(child, path))
				return found;
		}

		return nullptr;
	}

	ImGuiID AssetBrowserView::PathToId(const fs::path& path)
	{
		const size_t hash = fs::hash_value(path);
		return static_cast<ImGuiID>(hash ^ (hash >> 32));
	}

	ImGuiID AssetBrowserView::AdapterIndexToStorageId(ImGuiSelectionBasicStorage* self, i32 index)
	{
		auto* view = static_cast<AssetBrowserView*>(self->UserData);
		return PathToId(view->m_children[static_cast<usize>(index)]->GetSourcePath());
	}

	void AssetBrowserView::RenameOnDisk(const std::filesystem::path& sourcePath, const char* newName)
	{
		const fs::path newSource = sourcePath.parent_path() / (std::string(newName) + sourcePath.extension().string());

		if (fs::exists(newSource))
		{
			Terminal::Warn("AssetBrowserView", "target exists: {}", newSource.string());
			return;
		}

		fs::rename(sourcePath, newSource);

		fs::path oldMeta = sourcePath;
		oldMeta += ".hmeta";

		if (fs::exists(oldMeta))
		{
			fs::path newMeta = newSource;
			newMeta += ".hmeta";

			fs::rename(oldMeta, newMeta);
		}
	}
}