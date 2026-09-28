#include "SpinTestComponentDrawer.h"

#include <Runtime/Math/Scalar.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void SpinTestComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have mismatch!");
			return;
		}

		auto* pSpinComp = GetComponent<Engine::SpinTestComponent>();

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
		ImGui::TextUnformatted("Speed");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		ImGui::DragFloat("##enddist", &pSpinComp->m_speed, 0.1f, -FLT_MAX, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Axis");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		f32 rotArr[3] = { pSpinComp->m_axis.X(), pSpinComp->m_axis.Y(), pSpinComp->m_axis.Z() };
		if (ImGui::DragFloat3("##rotation", rotArr, 0.1f, -1, 1, "%.3f", ImGuiSliderFlags_ColorMarkers))
			pSpinComp->m_axis = { rotArr[0], rotArr[1], rotArr[2] };

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}