#pragma once

#include <Runtime/Graphics/RHI/Instance/GfxType.h>

#include <string>

namespace Horizon
{
	struct GfxInstanceDesc
	{
		std::string appName;
		Math::Vec3u appVer;
		GfxType type;

		GfxInstanceDesc& setAppName(const std::string& name) { appName = name; return *this; }
		GfxInstanceDesc& setAppVersion(const Math::Vec3u& version) { appVer = version; return *this; }
		GfxInstanceDesc& setAPIType(GfxType gfxType) { type = gfxType; return *this; }
	};
}
