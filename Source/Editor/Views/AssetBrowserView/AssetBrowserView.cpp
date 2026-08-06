#include "AssetBrowserView.h"

#include <imgui.h>

namespace Horizon
{
	static const char* CodepointToUtf8(unsigned int cp, char out[4])
	{
		out[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
		out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
		out[2] = (char)(0x80 | (cp & 0x3F));
		out[3] = 0;
		return out;
	}

	void AssetBrowserView::OnInvoke()
	{
	}

	void AssetBrowserView::OnRender()
	{
		ImFont* pFont = ImGui::GetFont();
		const ImGuiStyle& style = ImGui::GetStyle();
		const ImVec2 cell(ImGui::GetFontSize() * 2.0f, ImGui::GetFontSize() * 2.0f);
		const f32 wrapX = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

		i32 count = 0;
		for (u32 cp = 0xE000; cp <= 0xF8FF; ++cp)
		{
			if (!pFont->IsGlyphInFont((ImWchar)cp))
				continue;

			char icon[4];
			CodepointToUtf8(cp, icon);

			ImGui::PushID((i32)cp);
			if (ImGui::Button(icon, cell))
				ImGui::SetClipboardText(icon);
			ImGui::SetItemTooltip("U+%04X", cp);
			ImGui::PopID();

			if (ImGui::GetItemRectMax().x + style.ItemSpacing.x + cell.x < wrapX)
				ImGui::SameLine();
			++count;
		}

		ImGui::NewLine();
		ImGui::TextDisabled("%d glyph", count);
	}
}