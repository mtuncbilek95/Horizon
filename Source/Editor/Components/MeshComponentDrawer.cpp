#include "MeshComponentDrawer.h"

#include <Editor/Domain/DomainFile.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

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

		if (!ImGui::BeginTable("cameraComp", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Not Implemented Yet");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));

		/*if (ImGui::BeginDragDropTarget())
		{
			const ImGuiDragDropFlags acceptFlags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(nullptr, acceptFlags))
			{
				const DomainFile* givenFile = (DomainFile*)(pPayload->Data);
				Terminal::Warn(StringOps::GetName(this), "{} about to drop on MeshId", givenFile->GetName());

				accepting = true;

				if (pPayload->IsDelivery())
				{
					Terminal::Info(StringOps::GetName(this), "{} dropped on MeshId", givenFile->GetName());
				}
			}

			ImGui::EndDragDropTarget();
		}*/

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}