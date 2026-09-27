#include "MeshComponentDrawer.h"

#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Asset/AssetService.h>

#include <Runtime/Containers/Guid.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void MeshComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have mismatch!");
			return;
		}

		auto* pMeshComp = GetComponent<Engine::MeshComponent>();

		constexpr f32 cellMax = 80.0f;
		ImGuiStyle& style = ImGui::GetStyle();
		f32 spacing = style.ItemInnerSpacing.x;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));

		if (!ImGui::BeginTable("meshComp", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Mesh");
		ImGui::TableNextColumn();

		if (!pMeshComp->m_meshHandle.GetId().IsValid())
			m_currentLabel = "None";
		else
		{
			DomainFile* pFile = GetEngine()->RequestService<DomainService>()->FindFileByGuid(pMeshComp->m_meshHandle.GetId());
			m_currentLabel = pFile->GetPureName();
		}

		ImGui::Button(m_currentLabel.c_str(), ImVec2(-FLT_MIN, 0.f));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HZ_ASSET_FILE", ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				if (pPayload->DataSize != sizeof(DomainFile*))
					return;

				DomainFile* pDropped = *static_cast<DomainFile* const*>(pPayload->Data);
				const b8 acceptable = pDropped->GetMeta().assetTypeName == "MeshAsset";

				if (acceptable && pPayload->IsDelivery())
				{
					auto* pAssetService = GetEngine()->RequestService<Engine::AssetService>();

					m_currentLabel = pDropped->GetPureName();
					pMeshComp->m_meshHandle.SetId(pDropped->GetID());
					pMeshComp->m_meshHandle.SetAsset(pAssetService->FindAsset(pDropped->GetID()));

					Terminal::Info(StringOps::GetName(this), "{} dropped on MeshId", pDropped->GetID().ToString());
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}