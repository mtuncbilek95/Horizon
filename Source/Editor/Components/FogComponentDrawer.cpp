#include "FogComponentDrawer.h"

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Math/Scalar.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void FogComponentDrawer::OnRender()
	{
		if (GetComponent()->GetTypeId() != GetComponentId())
		{
			Terminal::Error(StringOps::GetName(this), "Somehow component types have mismatch!");
			return;
		}

		auto* pFogComp = GetComponent<Engine::FogComponent>();

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
		ImGui::TextUnformatted("Mode");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		auto* pReflection = GetEngine()->GetReflectionSystem();
		Reflect::Type* pFogModeType = pReflection->GetType(Reflect::TypeOf<Engine::FogMode>());
		
		std::span<Reflect::EnumValue const> enumValues = pFogModeType->GetEnumValues();

		const i64 currentValue = static_cast<i64>(pFogComp->m_mode);
		const c8* pPreview = "Unknown";

		for (const Reflect::EnumValue& enumValue : enumValues)
		{
			if (enumValue.value == currentValue)
			{
				pPreview = enumValue.name.c_str();
				break;
			}
		}

		if (ImGui::BeginCombo("##fogMode", pPreview))
		{
			for (const Reflect::EnumValue& enumValue : enumValues)
			{
				const b8 isSelected = enumValue.value == currentValue;

				if (ImGui::Selectable(enumValue.name.c_str(), isSelected))
					pFogComp->m_mode = static_cast<Engine::FogMode>(enumValue.value);

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Start Distance");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (pFogComp->m_mode == Engine::FogMode::Linear)
		{
			ImGui::DragFloat("##startdist", &pFogComp->m_startDistance, 0.1f, 0, pFogComp->m_endDistance, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("End Distance");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);

			ImGui::DragFloat("##enddist", &pFogComp->m_endDistance, 0.1f, pFogComp->m_startDistance + Math::KindaSmallNumber, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		}
		else
		{
			ImGui::DragFloat("##startdist", &pFogComp->m_startDistance, 0.1f, 0.f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Density");
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);

			ImGui::SliderFloat("##density", &pFogComp->m_density, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Max Opacity");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		ImGui::SliderFloat("##maxOpacity", &pFogComp->m_maxOpacity, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Fog Color");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);

		f32 fogClr[3] = { pFogComp->m_color.R(), pFogComp->m_color.G(), pFogComp->m_color.B() };
		ImGui::ColorEdit3("##fogColor", fogClr, ImGuiColorEditFlags_DisplayHex);
		pFogComp->m_color = Math::Color4f(fogClr[0], fogClr[1], fogClr[2], 1.f);

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}
}