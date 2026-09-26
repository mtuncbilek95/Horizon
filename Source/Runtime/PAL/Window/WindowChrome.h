#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>

namespace Horizon::PAL
{
	enum class ChromeButton : u8
	{
		None,
		Minimize,
		Maximize,
		Close
	};

	struct ChromeRect
	{
		i32 x = 0, y = 0, w = 0, h = 0;

		b8 Contains(i32 px, i32 py) const { return px >= x && py >= y && px < x + w && py < y + h; }
	};

	struct WindowChrome
	{
		u32 captionHeight = 0;
		u32 resizeBorder = 6;

		ChromeRect minimizeButton;
		ChromeRect maximizeButton;
		ChromeRect closeButton;

		List<ChromeRect> clientAreas;
	};
}