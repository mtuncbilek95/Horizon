#pragma once

#include <vector>

namespace Horizon
{
	class GfxImageView;
	class GfxRenderPass;

	struct GfxFramebufferDesc
	{
		GfxRenderPass* passRef = nullptr;
		std::vector<std::vector<GfxImageView*>> viewsPerFB;
		Math::Vec3u framebufferSize = { 1920, 1080, 1 };

		GfxFramebufferDesc& setPassRef(GfxRenderPass* pass) { passRef = pass; return *this; }
		GfxFramebufferDesc& addFramebufferViews(const std::vector<GfxImageView*>& views) { viewsPerFB.push_back(views); return *this; }
		GfxFramebufferDesc& setFramebufferSize(const Math::Vec3u& size) { framebufferSize = size; return *this; }
	};
}
