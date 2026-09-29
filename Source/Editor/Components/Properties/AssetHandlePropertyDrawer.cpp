#include "AssetHandlePropertyDrawer.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr const c8* sPickerPopup = "##assetPicker";
		static constexpr const c8* sClearPopup = "##assetClear";
	}

	b8 AssetHandlePropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Guid* pId = static_cast<Guid*>(pValue);
		const auto* pRef = field.GetCustomAttribute<Reflect::AssetRefAttribute>();

		if (!pRef)
		{
			ImGui::TextDisabled("%s", pId->IsValid() ? pId->ToString().c_str() : "None");
			return false;
		}

		Reflect::Type* pAssetType = ctx.pReflection->GetType(pRef->GetAssetType());

		if (!pAssetType)
		{
			Terminal::Error(StringOps::GetName(this), "{} references an unregistered asset type", field.GetName());
			return false;
		}

		auto* pDomain = ctx.pEngine->RequestService<DomainService>();
		const std::string& assetTypeName = pAssetType->GetName();

		std::string label = "None";

		if (pId->IsValid())
		{
			DomainFile* pFile = pDomain->FindFileByGuid(*pId);
			label = pFile ? pFile->GetPureName() : pId->ToString();
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

		changed |= DrawDropTarget(field, pId, assetTypeName);

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);

		if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS, ImVec2(pickerWidth, 0.f)))
		{
			m_search.clear();
			RefreshCandidates(pDomain, assetTypeName);
			ImGui::OpenPopup(sPickerPopup);
		}

		changed |= DrawPicker(pId, pDomain, assetTypeName);

		return changed;
	}

	b8 AssetHandlePropertyDrawer::DrawDropTarget(const Reflect::Field& field, Guid* pId, const std::string& assetTypeName)
	{
		if (!ImGui::BeginDragDropTarget())
			return false;

		b8 changed = false;

		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HZ_ASSET_FILE", ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (pPayload->DataSize == sizeof(DomainFile*))
			{
				DomainFile* pDropped = *static_cast<DomainFile* const*>(pPayload->Data);
				const b8 acceptable = pDropped->GetMeta().assetTypeName == assetTypeName;

				if (acceptable && pPayload->IsDelivery())
				{
					*pId = pDropped->GetID();
					changed = true;

					Terminal::Info(StringOps::GetName(this), "{} dropped on {}", pDropped->GetID().ToString(), field.GetName());
				}
			}
		}

		ImGui::EndDragDropTarget();
		return changed;
	}

	b8 AssetHandlePropertyDrawer::DrawPicker(Guid* pId, DomainService* pDomain, const std::string& assetTypeName)
	{
		ImGui::SetNextWindowSize(ImVec2(320.f, 360.f), ImGuiCond_Appearing);

		if (!ImGui::BeginPopup(sPickerPopup))
			return false;

		if (m_candidateRevision != pDomain->GetRevision() || m_candidateTypeName != assetTypeName)
			RefreshCandidates(pDomain, assetTypeName);

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
			if (!MatchesSearch(pFile->GetPureName(), m_search))
				continue;

			const b8 selected = pFile->GetID() == *pId;

			ImGui::PushID(pFile);

			if (ImGui::Selectable(pFile->GetPureName().c_str(), selected))
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

	void AssetHandlePropertyDrawer::RefreshCandidates(DomainService* pDomain, const std::string& assetTypeName)
	{
		m_candidates.Clear();
		m_candidateTypeName = assetTypeName;
		m_candidateRevision = pDomain->GetRevision();

		if (DomainFolder* pRoot = pDomain->GetRoot())
			CollectFiles(pRoot, assetTypeName);
	}

	void AssetHandlePropertyDrawer::CollectFiles(DomainFolder* pFolder, const std::string& assetTypeName)
	{
		for (DomainFile* pFile : pFolder->GetFiles())
		{
			if (pFile->GetMeta().assetTypeName == assetTypeName)
				m_candidates.PushBack(pFile);
		}

		for (DomainFolder* pChild : pFolder->GetFolders())
			CollectFiles(pChild, assetTypeName);
	}

	b8 AssetHandlePropertyDrawer::MatchesSearch(const std::string& name, const std::string& search)
	{
		if (search.empty())
			return true;

		if (search.size() > name.size())
			return false;

		for (usize start = 0; start + search.size() <= name.size(); start++)
		{
			usize i = 0;

			while (i < search.size() && std::tolower(static_cast<unsigned char>(name[start + i])) == std::tolower(static_cast<unsigned char>(search[i])))
				i++;

			if (i == search.size())
				return true;
		}

		return false;
	}
}