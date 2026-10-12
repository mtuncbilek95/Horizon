#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	class Window;

	class WindowListener
	{
	public:
		virtual ~WindowListener() = default;

		virtual void OnWindowFocusChanged(Window& window, b8 focused) {}
		virtual void OnWindowRectChanged(Window& window) {}
		virtual b8 OnWindowSetCursor(Window& window) { return false; }
		virtual void OnWindowDestroyed(Window& window) {}
	};
}