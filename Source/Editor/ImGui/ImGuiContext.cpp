#include "ImGuiContext.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon
{
	b8 DrawVector3(const std::string& label, f32& x, f32& y, f32& z, f32 resetValue, f32 columnWidth)
	{
		b8 changed = false;
		ImGuiStyle& style = ImGui::GetStyle();

		ImGui::PushID(label.data());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.data());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const f32 lineHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
		const ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		auto component = [&](const c8* axis, const c8* dragId, f32* value,
			const ImVec4& base, const ImVec4& hover)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, base);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, base);

				if (ImGui::Button(axis, buttonSize))
				{
					*value = resetValue;
					changed = true;
				}

				ImGui::PopStyleColor(3);
				ImGui::SameLine();

				if (ImGui::DragFloat(dragId, value, 0.1f, 0.0f, 0.0f, "%.2f"))
				{
					changed = true;
				}

				const ImVec2 rmin = ImGui::GetItemRectMin();
				const ImVec2 rmax = ImGui::GetItemRectMax();
				ImGui::GetWindowDrawList()->AddRectFilled(rmin, { rmin.x + 3.0f, rmax.y },
					ImGui::GetColorU32(base), style.FrameRounding);

				ImGui::PopItemWidth();
			};

		component("X", "##X", &x, { 0.80f, 0.15f, 0.20f, 1.0f }, { 0.92f, 0.24f, 0.28f, 1.0f });
		ImGui::SameLine();
		component("Y", "##Y", &y, { 0.22f, 0.55f, 0.24f, 1.0f }, { 0.30f, 0.68f, 0.32f, 1.0f });
		ImGui::SameLine();
		component("Z", "##Z", &z, { 0.15f, 0.30f, 0.80f, 1.0f }, { 0.22f, 0.40f, 0.92f, 1.0f });

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();

		return changed;
	}
}