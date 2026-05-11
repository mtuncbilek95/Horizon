#pragma once

#pragma once

#include <memory>

namespace Horizon
{
	class GfxImage;
	class GfxImageView;

	struct RenderTargetEntry
	{
		b8 isInUse = false;

		std::shared_ptr<GfxImage> img;
		std::shared_ptr<GfxImageView> view;
	};
}