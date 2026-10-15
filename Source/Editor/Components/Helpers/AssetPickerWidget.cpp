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

		changed |= DrawDropTarget(pId, pAssetType);

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);

		if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS, ImVec2(pickerWidth, 0.f)))
		{
			m_search.clear();
			RefreshCandidates(pDomain, pAssetType);
			ImGui::OpenPopup(sPickerPopup);
		}

		changed |= DrawPicker(pId, pDomain, pAssetType);

		return changed;
	}

	b8 AssetPickerWidget::DrawDropTarget(Guid* pId, Reflect::Type* pAssetType)
	{
		if (!ImGui::BeginDragDropTarget())
			return false;

		b8 changed = false;

		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HZ_ASSET_FILE", ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (pPayload->DataSize == sizeof(DomainFile*))
			{
				DomainFile* pDropped = *static_cast<DomainFile* const*>(pPayload->Data);
				const b8 acceptable = pDropped->GetAssetTypeName() == pAssetType->GetName();

				if (acceptable && pPayload->IsDelivery())
				{
					*pId = pDropped->GetID();
					changed = true;

					Terminal::Info("AssetPickerWidget", "{} dropped as {}", pDropped->GetID().ToString(), pAssetType->GetName());
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

		for (DomainFile* pFile : m_candidates)
		{
			if (!SearchFilter::Matches(pFile->GetName(), m_search))
				continue;

			const b8 selected = pFile->GetID() == *pId;

			ImGui::PushID(pFile);

			if (ImGui::Selectable(pFile->GetName().c_str(), selected))
			{
				*pId = pFile->GetID();
				changed = true;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", pFile->GetRelativePath().c_str());

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
			CollectFiles(pRoot, pAssetType);
	}

	void AssetPickerWidget::CollectFiles(DomainFolder* pFolder, Reflect::Type* pAssetType)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
		{
			if (pFile->GetAssetTypeName() == pAssetType->GetName())
				m_candidates.PushBack(pFile);
		}

		for (DomainFolder* pChild : pFolder->GetFolders())
			CollectFiles(pChild, pAssetType);
	}
}