#pragma once

namespace Horizon
{
	class GfxBuffer;
	class GfxPipeline;
	class GfxImageView;

	struct RenderableObject
	{
		GfxBuffer* meshData;
		GfxBuffer* materialData;

		std::vector<GfxImageView*> usedImages;

		GfxPipeline* usedPipeline;
		Math::Mat4f modelMatrix;
	};
}