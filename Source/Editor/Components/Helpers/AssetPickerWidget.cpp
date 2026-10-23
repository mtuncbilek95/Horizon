#include "AssetPickerWidget.h"

#include <Editor/Components/Helpers/SearchFilter.h>
#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cctype>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr const c8* sPickerPopup = "##assetPicker";
		static constexpr const c8* sClearPopup = "##assetClear";
	}

	b8 AssetPickerWidget::Draw(Guid* pId, Reflect::Type* pAssetType, DomainService* pDomain)
	{
		std::string label = "None";

		if (pId->IsValid())
		{
			DomainFile* pFile = pDomain->FindFileByGuid(*pId);
			label = pFile ? pFile->GetName() : pId->ToString();
		}

		const ImGuiStyle& style = ImGui::GetStyle();
		const f32 pickerWidth = ImGui::GetFrameHeight();
		const f32 dropWidth = ImGui::GetContentRegionAvail().x - pickerWidth - style.ItemInnerSpacing.x;

		b8 changed = false;

		ImGui::Button(label.c_str(), ImVec2(dropWidth, 0.f));

		if (ImGui::BeginPopupContextItem(sClearPopup))
		{
			if (ImGui::MenuItem("Clear"))
			{
				*pId = Guid();
				changed = true;
			}

			ImGui::EndPopup();
		}

		changed |= DrawDropTarget(pId, pAssetType, pDomain);

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);

		if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS, ImVec2(pickerWidth, 0.f)))
		{
			m_search.clear();
			m_openPickerRequest = true;
		}

		if (m_openPickerRequest)
		{
			RefreshCandidates(pDomain, pAssetType);
			ImGui::OpenPopup(sPickerPopup);
			m_openPickerRequest = false;
		}

		changed |= DrawPicker(pId, pDomain, pAssetType);

		return changed;
	}

	b8 AssetPickerWidget::DrawDropTarget(Guid* pId, Reflect::Type* pAssetType, DomainService* pDomain)
	{
		if (!ImGui::BeginDragDropTarget())
			return false;

		b8 changed = false;

		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HZ_ASSET_FILE"))
		{
			if (pPayload->DataSize == sizeof(DomainFile*))
			{
				DomainFile* pDropped = *static_cast<DomainFile* const*>(pPayload->Data);

				if (pDropped->GetAssetType() != pAssetType)
				{
					Terminal::Warn("AssetPickerWidget", "{} is not {}", pDropped->GetName(), pAssetType->GetName());
				}
				else
				{
					*pId = pDropped->GetID();
					changed = true;

					Terminal::Info("AssetPickerWidget", "{} dropped as {}", pDropped->GetName(), pAssetType->GetName());
				}
			}
		}

		ImGui::EndDragDropTarget();
		return changed;
	}

	b8 AssetPickerWidget::DrawPicker(Guid* pId, DomainService* pDomain, Reflect::Type* pAssetType)
	{
		ImGui::SetNextWindowSize(ImVec2(320.f, 360.f), ImGuiCond_Appearing);

		if (!ImGui::BeginPopup(sPickerPopup))
			return false;

		if (m_candidateRevision != pDomain->GetRevision() || m_candidateTypeName != pAssetType->GetName())
			RefreshCandidates(pDomain, pAssetType);

		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##search", "Search...", &m_search);
		ImGui::Separator();

		b8 changed = false;

		if (!ImGui::BeginChild("##list", ImVec2(0.f, 0.f), ImGuiChildFlags_None))
		{
			ImGui::EndChild();
			ImGui::EndPopup();
			return false;
		}

		if (m_search.empty())
		{
			if (ImGui::Selectable("None", !pId->IsValid()))
			{
				*pId = Guid();
				changed = true;
				ImGui::CloseCurrentPopup();
			}
		}

		for (const Candidate& candidate : m_candidates)
		{
			if (!SearchFilter::Matches(candidate.label, m_search))
				continue;

			const b8 selected = candidate.id == *pId;

			ImGui::PushID(candidate.id.ToString().c_str());

			if (ImGui::Selectable(candidate.label.c_str(), selected))
			{
				*pId = candidate.id;
				changed = true;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", candidate.tooltip.c_str());

			ImGui::PopID();
		}

		ImGui::EndChild();
		ImGui::EndPopup();

		return changed;
	}

	void AssetPickerWidget::RefreshCandidates(DomainService* pDomain, Reflect::Type* pAssetType)
	{
		m_candidates.Clear();
		m_candidateTypeName = pAssetType->GetName();
		m_candidateRevision = pDomain->GetRevision();

		if (DomainFolder* pRoot = pDomain->GetRoot())
			CollectCandidates(pRoot, pAssetType);
	}

	void AssetPickerWidget::CollectCandidates(DomainFolder* pFolder, Reflect::Type* pAssetType)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
		{
			if (pFile->GetAssetType() != pAssetType)
				continue;

			Candidate candidate;
			candidate.pFile = pFile;
			candidate.id = pFile->GetID();
			candidate.label = pFile->GetName();
			candidate.tooltip = pFile->GetRelativePath();
			m_candidates.PushBack(std::move(candidate));
		}

		for (DomainFolder* pChild : pFolder->GetFolders())
			CollectCandidates(pChild, pAssetType);
	}
}