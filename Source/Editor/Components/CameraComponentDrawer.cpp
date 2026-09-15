#include "CameraComponentDrawer.h"

#include <Runtime/Math/Scalar.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void CameraComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have mismatch!");
			return;
		}

		auto* pCamComp = GetComponent<Engine::CameraComponent>();

		constexpr f32 cellMax = 80.0f;
		const f32 spacing = ImGui::GetStyle().ItemInnerSpacing.x;

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
		ImGui::TextUnformatted("Field of View");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));
		ImGui::DragFloat("##fov", &pCamComp->m_fov, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Near Plane");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));
		ImGui::DragFloat("##nearPlane", &pCamComp->m_nearPlane, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Far Plane");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));
		ImGui::DragFloat("##farPlane", &pCamComp->m_farPlane, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers);


		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}