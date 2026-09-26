#include "TransformComponentDrawer.h"

#include <Runtime/Math/Scalar.h>
#include <imgui.h>

namespace Horizon::Editor
{
	void TransformComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have no match!");
			return;
		}

		auto* pTransformComp = GetComponent<Engine::TransformComponent>();

		constexpr f32 cellMax = 80.0f;
		const f32 spacing = ImGui::GetStyle().ItemInnerSpacing.x;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));

		if (!ImGui::BeginTable("transform", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		f32 posArr[3] = { pTransformComp->m_position.X(), pTransformComp->m_position.Y(), pTransformComp->m_position.Z() };
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Position");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));

		if (ImGui::DragFloat3("##position", posArr, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers))
			pTransformComp->m_position = { posArr[0], posArr[1], posArr[2] };

		f32 rotArr[3] = { Math::RadToDeg(pTransformComp->m_rotation.X()), Math::RadToDeg(pTransformComp->m_rotation.Y()), Math::RadToDeg(pTransformComp->m_rotation.Z()) };
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Rotation");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));

		if (ImGui::DragFloat3("##rotation", rotArr, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers))
			pTransformComp->m_rotation = { Math::DegToRad(rotArr[0]), Math::DegToRad(rotArr[1]), Math::DegToRad(rotArr[2]) };

		f32 sclArr[3] = { pTransformComp->m_scale.X(), pTransformComp->m_scale.Y(), pTransformComp->m_scale.Z() };
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Scale");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(std::min(ImGui::GetContentRegionAvail().x, cellMax * 3.0f + spacing * 2.0f));

		if (ImGui::DragFloat3("##scale", sclArr, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers))
			pTransformComp->m_scale = { sclArr[0], sclArr[1], sclArr[2] };

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}