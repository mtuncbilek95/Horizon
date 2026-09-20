#include "MeshComponentDrawer.h"

#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/Guid.h>

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <string>

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

		const Guid& currentId = pMeshComp->m_meshId.GetId();
		const std::string label = (currentId.IsValid() ? currentId.ToString() : std::string("None")) + "##meshId";
		const f32 width = std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f);

		ImGui::Button(label.c_str(), ImVec2(width, 0.0f));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("HZ_ASSET_MeshAsset"))
			{
				if (pPayload->DataSize == sizeof(Guid))
				{
					Guid droppedId;
					std::memcpy(&droppedId, pPayload->Data, sizeof(Guid));

					pMeshComp->m_meshId = Engine::AssetHandle<Engine::MeshAsset>(droppedId, nullptr);
					Terminal::Info(StringOps::GetName(this), "{} dropped on MeshId", droppedId.ToString());
				}
				else
				{
					Terminal::Error(StringOps::GetName(this), "Dropped payload is {} bytes, a Guid was expected", pPayload->DataSize);
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}