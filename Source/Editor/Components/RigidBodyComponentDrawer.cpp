#include "RigidBodyComponentDrawer.h"

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Math/Scalar.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void RigidBodyComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have mismatch!");
			return;
		}

		auto* pRBComp = GetComponent<Engine::RigidBodyComponent>();

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
		ImGui::TextUnformatted("Motion");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		auto* pReflection = GetEngine()->GetReflectionSystem();
		Reflect::Type* pPhysicsMotion = pReflection->GetType(Reflect::TypeOf<Engine::PhysicsMotion>());

		ReadOnlyList<Reflect::EnumValue const> enumValues = pPhysicsMotion->GetEnumValues();

		const i64 currentValue = static_cast<i64>(pRBComp->m_motion);
		const c8* pPreview = "Unknown";

		for (const Reflect::EnumValue& enumValue : enumValues)
		{
			if (enumValue.value == currentValue)
			{
				pPreview = enumValue.name.c_str();
				break;
			}
		}

		if (ImGui::BeginCombo("##motionMode", pPreview))
		{
			for (const Reflect::EnumValue& enumValue : enumValues)
			{
				const b8 isSelected = enumValue.value == currentValue;

				if (ImGui::Selectable(enumValue.name.c_str(), isSelected))
					pRBComp->m_motion = static_cast<Engine::PhysicsMotion>(enumValue.value);

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		f32 posArr[3] = { pRBComp->m_halfExtents.X(), pRBComp->m_halfExtents.Y(), pRBComp->m_halfExtents.Z() };
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Half Extends");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (ImGui::DragFloat3("##position", posArr, 0.1f, 0, 0, "%.3f", ImGuiSliderFlags_ColorMarkers))
			pRBComp->m_halfExtents = { posArr[0], posArr[1], posArr[2] };

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Mass");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		ImGui::DragFloat("##enddist", &pRBComp->m_mass, 0.1f, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}