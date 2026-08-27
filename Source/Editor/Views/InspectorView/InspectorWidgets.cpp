#include "InspectorWidgets.h"

#include <Editor/Attributes/TooltipAttribute.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 AxisColorX = IM_COL32(219, 62, 76, 255);
		constexpr u32 AxisColorY = IM_COL32(112, 184, 38, 255);
		constexpr u32 AxisColorZ = IM_COL32(41, 120, 219, 255);
	}

	void InspectorWidgets::BeginRow(const c8* pLabel, const Reflect::Field* pField)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(pLabel);

		auto* pTooltip = pField->GetCustomAttribute<TooltipAttribute>();

		if (pTooltip && ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", pTooltip->GetTooltip().c_str());

		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
	}

	b8 InspectorWidgets::DrawVec3(const c8* pLabel, f32* pValues, b8* pActive)
	{
		ImGui::PushID(pLabel);

		const f32 spacing = 6.0f;
		f32 available = ImGui::GetContentRegionAvail().x;
		f32 fieldWidth = (available - spacing * 2.0f) / 3.0f;

		b8 changed = false;
		b8 active = false;

		changed |= DrawAxisField("X", &pValues[0], AxisColorX, fieldWidth);
		active |= ImGui::IsItemActive();

		ImGui::SameLine(0.0f, spacing);

		changed |= DrawAxisField("Y", &pValues[1], AxisColorY, fieldWidth);
		active |= ImGui::IsItemActive();

		ImGui::SameLine(0.0f, spacing);

		changed |= DrawAxisField("Z", &pValues[2], AxisColorZ, fieldWidth);
		active |= ImGui::IsItemActive();

		ImGui::PopID();

		if (pActive)
			*pActive = active;

		return changed;
	}

	b8 InspectorWidgets::DrawAxisField(const c8* pAxis, f32* pValue, u32 accentColor, f32 width)
	{
		ImGui::PushID(pAxis);

		ImDrawList* pDraw = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();

		const f32 height = ImGui::GetFrameHeight();
		const f32 rounding = ImGui::GetStyle().FrameRounding;
		const f32 stripWidth = 3.0f;
		const f32 inset = height * 0.85f;

		ImVec2 corner = ImVec2(origin.x + width, origin.y + height);
		pDraw->AddRectFilled(origin, corner, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);

		pDraw->PushClipRect(origin, ImVec2(origin.x + stripWidth, corner.y), true);
		pDraw->AddRectFilled(origin, corner, accentColor, rounding);
		pDraw->PopClipRect();

		ImVec2 axisSize = ImGui::CalcTextSize(pAxis);
		ImVec2 axisPos = ImVec2(origin.x + stripWidth + (inset - stripWidth - axisSize.x) * 0.5f,
			origin.y + (height - axisSize.y) * 0.5f);

		pDraw->AddText(axisPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), pAxis);

		ImGui::SetCursorScreenPos(ImVec2(origin.x + inset, origin.y));
		ImGui::SetNextItemWidth(width - inset);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, 0);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, 0);

		b8 changed = ImGui::DragFloat("##value", pValue, 0.01f, 0.0f, 0.0f, "%.3f");

		ImGui::PopStyleColor(3);

		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			pDraw->AddRect(origin, corner, accentColor, rounding, 0, 1.0f);

		ImGui::PopID();

		return changed;
	}
}