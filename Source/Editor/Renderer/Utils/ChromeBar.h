#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Window/WindowChrome.h>

#include <imgui.h>

namespace Horizon::PAL
{
	class Window;
}

namespace Horizon::Editor
{
	struct EDITOR_API ChromeBar final
	{
		static constexpr f32 Height = 32.0f;
		static constexpr f32 ButtonWidth = 46.0f;
		static constexpr f32 TitlePadding = 12.0f;

		static f32 GetButtonsWidth(b8 closable);
		static PAL::ChromeRect ToChromeRect(const ImVec2& min, const ImVec2& max);

		static PAL::ChromeRect RenderButton(const c8* pLabel, b8 danger);
		static void RenderButtons(PAL::Window* pWindow, PAL::WindowChrome& chrome, b8 closable);
		static PAL::ChromeRect RenderTitleHandle(const c8* pTitle);
	};
}