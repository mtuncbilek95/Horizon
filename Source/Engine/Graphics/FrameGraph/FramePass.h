#pragma once

#include <Runtime/Graphics/GfxBackend.h>

#include <string_view>
#include <functional>

namespace Horizon
{
	struct FramePass
	{
		std::string_view name;
		std::function<void(GfxCmdList*)> execute;
	};
}