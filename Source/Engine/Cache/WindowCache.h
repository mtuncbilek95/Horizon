#pragma once

#include <Runtime/Window/Window.h>

namespace Horizon
{
	struct WindowCache
	{
		std::string name = "Horizon Test";
		Math::Vec2u windowSize = { 1920, 1080 };
		WindowMode mode = WindowMode::Windowed;
	};
}