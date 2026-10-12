#include "ChromeBar.h"

#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Runtime/PAL/Window/Window.h>

#include <imgui_internal.h>

namespace Horizon::Editor
{
	f32 ChromeBar::GetButtonsWidth(b8 closable)
	{
		return ButtonWidth * (closable ? 3.0f : 2.0f);
	}

	PAL::ChromeRect ChromeBar::ToChromeRect(const ImVec2& min, const ImVec2& max)
	{
		const ImVec2 origin = ImGui::GetWindowViewport()->Pos;

		PAL::ChromeRect rect = {};
		rect.x = i32(min.x - origin.x);
		rect.y = i32(min.y - origin.y);
		rect.w = i32(max.x - min.x);
		rect.h = i32(max.y - min.y);

		return rect;
	}

	PAL::ChromeRect ChromeBar::RenderButton(const c8* pLabel, b8 danger)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImGuiUtils::Hex("#00000000"));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, danger ? ImGuiUtils::Hex("#C42B1C") : ImGuiUtils::Hex("#3C3C3C"));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, danger ? ImGuiUtils::Hex("#E81123") : ImGuiUtils::Hex("#4A4A4A"));

		ImGui::Button(pLabel, ImVec2(ButtonWidth, Height));

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);

		return ToChromeRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	void ChromeBar::RenderButtons(PAL::Window* pWindow, PAL::WindowChrome& chrome, b8 closable)
	{
		const c8* pMaximizeLabel = pWindow->GetMaximized() ?
			ICON_FA_WINDOW_RESTORE "##Maximize" : ICON_FA_WINDOW_MAXIMIZE "##Maximize";

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		chrome.minimizeButton = RenderButton(ICON_FA_WINDOW_MINIMIZE "##Minimize", false);
		ImGui::SameLine(0.0f, 0.0f);
		chrome.maximizeButton = RenderButton(pMaximizeLabel, false);

		if (closable)
		{
			ImGui::SameLine(0.0f, 0.0f);
			chrome.closeButton = RenderButton(ICON_FA_XMARK "##Close", true);
		}

		ImGui::PopStyleVar();
	}

	PAL::ChromeRect ChromeBar::RenderTitleHandle(const c8* pTitle)
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1], 0.0f);

		const ImVec2 textSize = ImGui::CalcTextSize(pTitle);
		const ImVec2 handleSize = ImVec2(textSize.x + TitlePadding * 2.0f, Height);
		const ImVec2 handlePos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton("##TitleHandle", handleSize);

		if (ImGui::IsItemActivated())
		{
			ImGuiWindow* pWindow = ImGui::GetCurrentWindow();
			ImGui::StartMouseMovingWindow(pWindow);
			ImGui::GetCurrentContext()->MovingWindow = pWindow;
		}

		const ImVec2 textPos = ImVec2(handlePos.x + TitlePadding, handlePos.y + (Height - textSize.y) * 0.5f);
		ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), pTitle);

		ImGui::PopFont();

		return ToChromeRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}
}