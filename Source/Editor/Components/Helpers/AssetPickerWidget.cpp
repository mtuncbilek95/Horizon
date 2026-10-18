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
			const DomainFile::DomainSubAsset* pSubAsset = nullptr;
			DomainFile* pFile = pDomain->FindFileBySubAssetId(*pId, &pSubAsset);

			if (!pFile)
				label = pId->ToString();
			else if (pSubAsset->key.empty())
				label = pFile->GetName();
			else
				label = pFile->GetName() + " / " + pSubAsset->key;
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

				List<Candidate> matches;
				CollectProducts(pDropped, pAssetType, matches);

				if (matches.GetCount() > 0 && pPayload->IsDelivery())
				{
					if (matches.GetCount() == 1)
					{
						*pId = matches[0].id;
						changed = true;

						Terminal::Info("AssetPickerWidget", "{} dropped as {}", matches[0].label, pAssetType->GetName());
					}
					else
					{
						m_search = pDropped->GetName();
						m_openPickerRequest = true;
					}
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
			CollectProducts(pRoot, pAssetType);
	}

	void AssetPickerWidget::CollectProducts(DomainFolder* pFolder, Reflect::Type* pAssetType)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
			CollectProducts(pFile, pAssetType, m_candidates);

		for (DomainFolder* pChild : pFolder->GetFolders())
			CollectProducts(pChild, pAssetType);
	}

	void AssetPickerWidget::CollectProducts(DomainFile* pFile, Reflect::Type* pAssetType, List<Candidate>& outCandidates) const
	{
		for (const DomainFile::DomainSubAsset& subAsset : pFile->GetSubAssets())
		{
			if (subAsset.pAssetType != pAssetType)
				continue;

			Candidate candidate;
			candidate.pFile = pFile;
			candidate.id = subAsset.id;
			candidate.label = subAsset.key.empty() ? pFile->GetName() : pFile->GetName() + " / " + subAsset.key;
			candidate.tooltip = pFile->GetRelativePath() + (subAsset.key.empty() ? std::string() : " / " + subAsset.key);
			outCandidates.PushBack(std::move(candidate));
		}
	}
}