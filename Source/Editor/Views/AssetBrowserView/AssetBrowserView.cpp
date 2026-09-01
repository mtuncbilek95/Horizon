#include "AssetBrowserView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Domain/DomainService.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>

#include <imgui_internal.h>

namespace Horizon::Editor
{
	namespace
	{
		ImGuiID HashPath(const std::string& path)
		{
			return static_cast<ImGuiID>(std::hash<std::string>{}(path));
		}

		std::string TruncateToWidth(const std::string& text, f32 maxWidth)
		{
			if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
				return text;

			const f32 ellipsisWidth = ImGui::CalcTextSize("...").x;

			std::string result;
			f32 width = 0.f;

			for (const c8& character : text)
			{
				const f32 charWidth = ImGui::CalcTextSize(&character, &character + 1).x;

				if (width + charWidth + ellipsisWidth > maxWidth)
					break;

				result.push_back(character);
				width += charWidth;
			}

			return result + "...";
		}
	}

	const std::string& AssetBrowserView::BrowserEntry::GetPath() const
	{
		return pFolder ? pFolder->GetAbsolutePath() : pFile->GetSourcePath();
	}

	const std::string& AssetBrowserView::BrowserEntry::GetName() const
	{
		return pFolder ? pFolder->GetName() : pFile->GetName();
	}

	void AssetBrowserView::OnInvoke()
	{
		auto* pDomain = GetContext()->pEngine->RequestService<DomainService>();
		m_currentFolder = pDomain->GetRoot();

		m_contextMenu.BootstrapContext(GetContext()->pEngine, "AssetBrowserView");
	}

	void AssetBrowserView::OnRender()
	{
		if (m_currentFolder == nullptr)
		{
			ImGui::TextDisabled("Root folder is unavailable");
			return;
		}

		DomainFolder* pFolder = m_currentFolder;

		RenderToolbar(pFolder);
		ImGui::Separator();

		RebuildEntries(pFolder);
		RenderGrid();

		AssetBrowserContext context = {};
		context.pEngine = GetContext()->pEngine;
		context.currentFolder = pFolder;

		CollectSelected(context);

		m_contextMenu.RenderGUI("AssetBrowserContextMenu", context);

		if (!context.renamePath.empty())
			BeginRename(context.renamePath);

		RenderRenameModal();
	}

	void AssetBrowserView::RenderToolbar(DomainFolder* pFolder)
	{
		ImGui::BeginDisabled(pFolder->IsRoot());

		if (ImGui::Button(ICON_FA_ARROW_LEFT) && !pFolder->IsRoot())
			Navigate(pFolder->GetParent());

		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();

		List<const std::string*> chain;
		DomainFolder* pWalk = pFolder;

		while (pWalk != nullptr)
		{
			chain.PushBack(&pWalk->GetName());
			pWalk = pWalk->GetParent();
		}

		chain.ReverseOrder();

		std::string display;

		for (const std::string* pName : chain)
			display += *pName + "/";

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		ImGui::TextUnformatted(display.c_str());
		ImGui::PopStyleColor();
	}

	void AssetBrowserView::RebuildEntries(DomainFolder* pFolder)
	{
		m_entries.Clear();

		for (DomainFolder* pSubFolder : pFolder->GetFolders())
		{
			BrowserEntry& entry = m_entries.EmplaceBack(pSubFolder, nullptr, 0);
			entry.id = HashPath(entry.GetPath());
		}

		for (DomainFile* pFile : pFolder->GetFiles())
		{
			BrowserEntry& entry = m_entries.EmplaceBack(nullptr, pFile, 0);
			entry.id = HashPath(entry.GetPath());
		}
	}

	void AssetBrowserView::RenderGrid()
	{
		const f32 cell = ImGui::GetFontSize() * 4.f;
		const f32 step = cell + ImGui::GetStyle().ItemSpacing.x;
		const usize columnCount = static_cast<usize>(ImMax(1, static_cast<i32>(ImGui::GetContentRegionAvail().x / step)));
		const ImVec2 startPos = ImGui::GetCursorScreenPos();

		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_NavWrapX;

		if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			msFlags |= ImGuiMultiSelectFlags_BoxSelect2d | ImGuiMultiSelectFlags_ClearOnClickVoid;

		ImGuiMultiSelectIO* pMultiIO = ImGui::BeginMultiSelect(msFlags, m_selection.Size, static_cast<i32>(m_entries.GetCount()));

		m_selection.UserData = this;
		m_selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* pSelf, int index)
			{
				AssetBrowserView* pView = static_cast<AssetBrowserView*>(pSelf->UserData);
				return pView->m_entries[static_cast<usize>(index)].id;
			};

		m_selection.ApplyRequests(pMultiIO);

		DomainFolder* pEnterFolder = nullptr;
		DomainFile* pOpenFile = nullptr;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		for (usize i = 0; i < m_entries.GetCount(); i++)
		{
			const BrowserEntry& entry = m_entries[i];
			const ImVec2 cellMin = ImVec2(startPos.x + (i % columnCount) * step, startPos.y + (i / columnCount) * step);

			ImGui::SetCursorScreenPos(cellMin);
			ImGui::PushID(static_cast<i32>(entry.id));

			ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(i));
			ImGui::Selectable("", m_selection.Contains(entry.id), ImGuiSelectableFlags_None, ImVec2(cell, cell));

			if (entry.IsFolder() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				pEnterFolder = entry.pFolder;

			if (!entry.IsFolder() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				pOpenFile = entry.pFile;

			DrawCell(pDrawList, entry, cellMin, cell);

			ImGui::PopID();
		}

		pMultiIO = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(pMultiIO);

		const usize lineCount = (m_entries.GetCount() + columnCount - 1) / columnCount;
		ImGui::SetCursorScreenPos(startPos);
		ImGui::Dummy(ImVec2(columnCount * step, lineCount * step));

		if (pEnterFolder != nullptr)
			Navigate(pEnterFolder);

		// TODO: Open file
		//if (pOpenFile != nullptr)
			
	}

	void AssetBrowserView::DrawCell(ImDrawList* pDrawList, const BrowserEntry& entry, const ImVec2& cellMin, f32 cell)
	{
		const c8* pIcon = entry.IsFolder() ? ICON_FA_FOLDER : ICON_FA_FILE;
		const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);

		ImGui::PushFont(ImGui::GetFont(), cell * 0.5f);

		const ImVec2 iconSize = ImGui::CalcTextSize(pIcon);
		const ImVec2 iconPos = ImVec2(cellMin.x + (cell - iconSize.x) * 0.5f, cellMin.y + cell * 0.14f);

		pDrawList->AddText(iconPos, textColor, pIcon);
		ImGui::PopFont();

		const std::string label = TruncateToWidth(entry.GetName(), cell - 6.f);
		const ImVec2 nameSize = ImGui::CalcTextSize(label.c_str());
		const ImVec2 namePos = ImVec2(cellMin.x + (cell - nameSize.x) * 0.5f, cellMin.y + cell - nameSize.y - 4.f);

		pDrawList->AddText(namePos, textColor, label.c_str());

		if (label != entry.GetName() && ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", entry.GetName().c_str());
	}

	void AssetBrowserView::CollectSelected(AssetBrowserContext& context)
	{
		for (const BrowserEntry& entry : m_entries)
		{
			if (!m_selection.Contains(entry.id))
				continue;

			if (entry.IsFolder())
				context.selectedFolders.PushBack(entry.pFolder);
			else
				context.selectedFiles.PushBack(entry.pFile);
		}
	}

	void AssetBrowserView::Navigate(DomainFolder* pTarget)
	{
		if (pTarget == nullptr)
			return;

		m_currentFolder = pTarget;
		m_selection.Clear();
	}

	void AssetBrowserView::BeginRename(const std::string& sourcePath)
	{
		if (!m_renamePath.empty())
			return;

		const BrowserEntry* pEntry = FindEntry(sourcePath);

		if (pEntry == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a tracked browser entry", sourcePath);
			return;
		}

		m_renamePath = sourcePath;
		m_renameIsFolder = pEntry->IsFolder();

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pEntry->GetName().c_str());
		ImGui::OpenPopup("Rename - Asset Browser");
	}

	void AssetBrowserView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal("Rename - Asset Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			auto* entry = FindEntry(m_renamePath);
			entry->IsFolder() ? entry->pFolder->Rename(m_renameBuffer) : entry->pFile->Rename(m_renameBuffer);
			m_renamePath.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_renamePath.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	AssetBrowserView::BrowserEntry* AssetBrowserView::FindEntry(const std::string& path)
	{
		for (auto& entry : m_entries)
		{
			if (entry.GetPath() == path)
				return &entry;
		}

		return nullptr;
	}
}