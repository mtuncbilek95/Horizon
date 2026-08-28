#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief How the presentation engine paces the frames.
	 * Immediate tears but never blocks, Vsync waits for
	 * the display refresh.
	 *
	 * @code
	 *   RHI::GfxSwapchainDesc chainDesc = {};
	 *   chainDesc.presentMode = RHI::GfxPresentMode::Vsync;
	 * @endcode
	 */
	enum class GfxPresentMode : u8
	{
		Immediate,
		Vsync
	};
}
