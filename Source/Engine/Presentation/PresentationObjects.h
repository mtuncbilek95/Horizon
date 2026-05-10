#pragma once

namespace Horizon
{
	class GfxImageView;
	class GfxCommandBuffer;

	struct CompositePresentObject
	{
		u32 frameIndex;
		u32 imageIndex;
		GfxImageView* imageView;

		CompositePresentObject& setFrameIndex(u32 idx) { frameIndex = idx; return *this; }
		CompositePresentObject& setImageIndex(u32 idx) { imageIndex = idx; return *this; }
		CompositePresentObject& setImageView(GfxImageView* view) { imageView = view; return *this; }
	};

	struct CompositeSubmitObject
	{
		u32 frameIndex;
		u32 imageIndex;
		std::vector<GfxCommandBuffer*> cmdBuffers;

		CompositeSubmitObject& setFrameIndex(u32 idx) { frameIndex = idx; return *this; }
		CompositeSubmitObject& setImageIndex(u32 idx) { imageIndex = idx; return *this; }
		CompositeSubmitObject& addCmdBuffer(GfxCommandBuffer* bf) { cmdBuffers.push_back(bf); return *this; }
	};
}