#include "AssetBrowserView.h"

#include <imgui.h>

namespace Horizon
{
	constexpr ImVec4 kAxisX = ImVec4(0.659f, 0.408f, 0.431f, 1.0f); // #A8686E
	constexpr ImVec4 kAxisY = ImVec4(0.455f, 0.631f, 0.514f, 1.0f); // #74A183
	constexpr ImVec4 kAxisZ = ImVec4(0.431f, 0.561f, 0.710f, 1.0f); // #6E8FB5

	b8 AxisField(const char* id, f32* v, const ImVec4& tint, f32 width,
		const char* fmt, f32 speed)
	{
		ImGui::SetNextItemWidth(width);
		const b8 changed = ImGui::DragFloat(id, v, speed, 0.0f, 0.0f, fmt);

		const ImVec2 mn = ImGui::GetItemRectMin();
		const ImVec2 mx = ImGui::GetItemRectMax();
		const f32 stripW = 3.0f;

		ImGui::GetWindowDrawList()->AddRectFilled(
			mn, ImVec2(mn.x + stripW, mx.y), ImGui::GetColorU32(tint), 0.0f);

		return changed;
	}

	b8 VectorRow(const char* label, f32 v[3], const char* fmt = "%.2f",
		f32 speed = 0.01f)
	{
		ImGui::PushID(label);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);

		ImGui::TableSetColumnIndex(1);
		ImGui::AlignTextToFramePadding();
		const b8 menu = ImGui::ArrowButton("##opts", ImGuiDir_Down);
		if (menu) ImGui::OpenPopup("##ctx");

		ImGui::TableSetColumnIndex(2);
		const f32 avail = ImGui::GetContentRegionAvail().x;
		const f32 sp = ImGui::GetStyle().ItemSpacing.x;
		const f32 w = (avail - sp * 2.0f) / 3.0f;

		b8 changed = false;
		changed |= AxisField("##x", &v[0], kAxisX, w, fmt, speed);
		ImGui::SameLine(0.0f, sp);
		changed |= AxisField("##y", &v[1], kAxisY, w, fmt, speed);
		ImGui::SameLine(0.0f, sp);
		changed |= AxisField("##z", &v[2], kAxisZ, w, fmt, speed);

		if (ImGui::BeginPopup("##ctx"))
		{
			if (ImGui::MenuItem("Reset")) { v[0] = v[1] = v[2] = 0.0f; changed = true; }
			ImGui::EndPopup();
		}

		ImGui::PopID();
		return changed;
	}

	void DrawTransform(f32 loc[3], f32 rot[3], f32 scale[3])
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 2.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

		const b8 open = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);

		if (open)
		{
			constexpr ImGuiTableFlags flags = ImGuiTableFlags_BordersInner
				| ImGuiTableFlags_RowBg
				| ImGuiTableFlags_SizingFixedFit;

			if (ImGui::BeginTable("##transform", 3, flags))
			{
				ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
				ImGui::TableSetupColumn("##arrow", ImGuiTableColumnFlags_WidthFixed, 20.0f);
				ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

				VectorRow("Location", loc, "%.2f", 0.01f);
				VectorRow("Rotation", rot, "%.2f\xC2\xB0", 0.10f);
				VectorRow("Scale", scale, "%.2f", 0.01f);

				ImGui::EndTable();
			}
		}

		ImGui::PopStyleVar(3);
	}

	void AssetBrowserView::OnInvoke()
	{
	}

	void AssetBrowserView::OnRender()
	{
		f32 loc[3];
		f32 pos[3];
		f32 sca[3];

		DrawTransform(loc, pos, sca);
	}
}