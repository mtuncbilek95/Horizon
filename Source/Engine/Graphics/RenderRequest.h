#pragma once

#include <functional>

namespace Horizon
{
	class GfxCommandBuffer;

	struct RenderRequest
	{
		std::function<void(GfxCommandBuffer*)> command;
	};
}