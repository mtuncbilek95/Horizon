#include "AssetBrowserWidget.h"

#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuRegistry.h>
#include <Editor/Domain/DomainSystem.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserContext.h>

#include <Engine/Core/Engine.h>

#include <cstdio>

namespace Horizon
{
	AssetBrowserWidget::~AssetBrowserWidget()
	{
		Allocator::Delete(m_menuRegistry);
	}

	void AssetBrowserWidget::OnInvoke()
	{
		if (!m_menuRegistry)
			m_menuRegistry = Allocator::Create<AssetBrowserMenuRegistry>(CurrLoc(), GetEngine());

		m_menuRegistry->Invalidate();

		auto& domainSub = GetEngine()->GetSystem<DomainSystem>();

		if (!m_currentFolder)
			m_currentFolder = domainSub.GetRootFolder();
	}

	void AssetBrowserWidget::OnDraw()
	{
		if (!m_currentFolder)
			return;

		const auto& folders = m_currentFolder->GetSubFolders();
		const auto& files = m_currentFolder->GetFiles();
		const i32 itemCount = (i32)(folders.size() + files.size());

		if (ImGui::Button(ICON_FA_ARROW_LEFT))
			m_currentFolder = m_currentFolder->IsRoot() ? m_currentFolder : m_currentFolder->GetParentFolder();

		ImGui::SameLine();

		ImGui::TextUnformatted(m_currentFolder->GetName().c_str());
		ImGui::Separator();

		DomainFolder* navigateTo = nullptr;
		b8 openContext = false;
		i32 rightClickIndex = -1;

		ImGui::BeginChild("AssetBrowserBody");

		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;
		ImGuiMultiSelectIO* io = ImGui::BeginMultiSelect(msFlags, m_selection.Size, itemCount);
		m_selection.ApplyRequests(io);

		i32 index = 0;

		// Folders
		for (DomainFolder* folder : folders)
		{
			ImGui::PushID(index);
			ImGui::SetNextItemSelectionUserData(index);

			b8 selected = m_selection.Contains((ImGuiID)index);

			c8 label[256];
			std::snprintf(label, sizeof(label), "%s  %s", ICON_FA_FOLDER, folder->GetName().c_str());
			ImGui::Selectable(label, selected, ImGuiSelectableFlags_AllowDoubleClick);

			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					navigateTo = folder;
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					rightClickIndex = index;
					openContext = true;
				}
			}

			ImGui::PopID();
			index++;
		}

		for (DomainFile* file : files)
		{
			ImGui::PushID(index);
			ImGui::SetNextItemSelectionUserData(index);

			b8 selected = m_selection.Contains((ImGuiID)index);

			c8 label[256];
			std::snprintf(label, sizeof(label), "%s  %s", ICON_FA_FILE, file->GetName().c_str());
			ImGui::Selectable(label, selected, ImGuiSelectableFlags_AllowDoubleClick);

			if (ImGui::IsItemHovered())
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					// TODO: ActivateAsset(file) — sen ekleyeceksin
				}
				else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					rightClickIndex = index;
					openContext = true;
				}
			}

			ImGui::PopID();
			index++;
		}

		if (!openContext && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
			ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			openContext = true;
			rightClickIndex = -1;
		}

		io = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(io);

		ImGui::EndChild();

		if (openContext)
		{
			if (rightClickIndex >= 0 && !m_selection.Contains((ImGuiID)rightClickIndex))
			{
				m_selection.Clear();
				m_selection.SetItemSelected((ImGuiID)rightClickIndex, true);
			}
			else if (rightClickIndex < 0)
			{
				m_selection.Clear();
			}

			m_menuRegistry->Open();
		}

		std::vector<DomainFile*> selectedFiles;
		for (usize f = 0; f < files.size(); ++f)
		{
			i32 fileIndex = (i32)(folders.size() + f);
			if (m_selection.Contains((ImGuiID)fileIndex))
				selectedFiles.push_back(files[f]);
		}

		AssetBrowserContext ctx{ m_currentFolder, std::move(selectedFiles) };
		m_menuRegistry->Render(ctx);

		if (navigateTo)
		{
			m_currentFolder = navigateTo;
			m_selection.Clear();
		}
	}
}