#pragma once

#include <Runtime/Graphics/RHI/Util/PresentMode.h>
#include <Runtime/Graphics/RHI/Instance/GfxType.h>

namespace Horizon
{
	struct GfxCache
	{
		std::string appName = "Horizon Test";
		std::string engineName = "Horizon";
		Math::Vec3u appVersion = { 1, 0, 0 };
		u32 imageCount = 3;
		PresentMode presentType = PresentMode::Mailbox;
		GfxType rhiType = GfxType::Vulkan;
	};
}