#include "AssetBrowserWidget.h"

#include <Editor/Domain/DomainSystem.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/DomainFolder.h>

#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/Core/Engine.h>
#include <Engine/ECS/EntityComponentSystem.h>
#include <Engine/Asset/AssetSystem.h>
#include <Engine/Asset/IAsset.h>
#include <Engine/Asset/Loaders/World/WorldAsset.h>

namespace Horizon
{
	namespace
	{
		static const char* IconForFile(DomainFile* pFile)
		{
			return ICON_FA_FILE;
		}
	}

	void AssetBrowserWidget::OnInvoke()
	{
		auto& domainSub = GetEngine()->GetSystem<DomainSystem>();
		m_currentFolder = domainSub.GetRootFolder();
	}

	void AssetBrowserWidget::OnDraw()
	{
		if (!m_currentFolder)
			return;

		const auto& subfolders = m_currentFolder->GetSubfolders();
		const auto& files = m_currentFolder->GetFiles();
		i32 itemCount = (i32)(subfolders.size() + files.size());

		DomainFolder* navigateTo = nullptr;
		DomainFile* activateFile = nullptr;

		b8 isRoot = m_currentFolder->IsRoot();
		if (isRoot)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_ARROW_LEFT))
			navigateTo = m_currentFolder->GetParentFolder();

		if (isRoot)
			ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::TextUnformatted(m_currentFolder->GetRelativePath().string().c_str());

		ImGui::Separator();

		b8 openContext = false;

		if (ImGui::BeginChild("AssetBrowserBody"))
		{
			ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d;

			ImGuiMultiSelectIO* pIo = ImGui::BeginMultiSelect(flags, m_selection.Size, itemCount);
			m_selection.ApplyRequests(pIo);

			i32 index = 0;

			for (DomainFolder* pFolder : subfolders)
			{
				char label[256];
				std::snprintf(label, sizeof(label), "%s  %s", ICON_FA_FOLDER, pFolder->GetName().c_str());

				b8 selected = m_selection.Contains((ImGuiID)index);
				ImGui::SetNextItemSelectionUserData(index);
				ImGui::Selectable(label, selected, ImGuiSelectableFlags_AllowDoubleClick);

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					navigateTo = pFolder;

				index++;
			}

			for (DomainFile* pFile : files)
			{
				char label[256];
				std::snprintf(label, sizeof(label), "%s  %s", IconForFile(pFile), pFile->GetName().c_str());

				b8 selected = m_selection.Contains((ImGuiID)index);
				ImGui::SetNextItemSelectionUserData(index);
				ImGui::Selectable(label, selected, ImGuiSelectableFlags_AllowDoubleClick);

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					activateFile = pFile;

				index++;
			}

			pIo = ImGui::EndMultiSelect();
			m_selection.ApplyRequests(pIo);

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				openContext = true;
		}
		ImGui::EndChild();

		// ======================= TEST CONTEXT MENU AREA ======================= //
#if (USE_DEV_MODE)
		if (openContext)
			ImGui::OpenPopup("AssetBrowserContext");

		if (ImGui::BeginPopup("AssetBrowserContext"))
		{
			if (ImGui::MenuItem("Create New Folder"))
			{
				auto& domainSub = GetEngine()->GetSystem<DomainSystem>();
				domainSub.AddNewFolder(m_currentFolder);
			}

			if (ImGui::MenuItem("Create Empty Scene"))
			{
				auto& domainSub = GetEngine()->GetSystem<DomainSystem>();
				domainSub.ImportDefault(m_currentFolder, ".hworld");
			}

			ImGui::EndPopup();
		}
#else // Use actual version
#endif

		if (activateFile)
			ActivateAsset(activateFile);

		if (navigateTo)
		{
			m_currentFolder = navigateTo;
			m_selection.Clear();
		}
	}

	void AssetBrowserWidget::ActivateAsset(DomainFile* pFile)
	{
		auto* assetSub = GetEngine()->TryGetSystem<AssetSystem>();
		if (!assetSub)
		{
			Terminal::Warn("AssetBrowser", "No AssetSystem");
			return;
		}

		IAsset* asset = assetSub->Load(pFile->GetGuid());
		if (!asset)
		{
			Terminal::Warn("AssetBrowser", "Failed to load '{}' (guid {})",
				pFile->GetName(), pFile->GetGuid().ToString());
			return;
		}

		if (WorldAsset* world = dynamic_cast<WorldAsset*>(asset))
		{
			auto* ecs = GetEngine()->TryGetSystem<EntityComponentSystem>();
			if (!ecs)
				return;

			ecs->SetActiveWorld(world);
			Terminal::Log("AssetBrowser", "Activated world '{}'", world->GetWorldName());
			return;
		}

		Terminal::Warn("AssetBrowser", "'{}' is not a World — nothing to activate", pFile->GetName());
	}
}