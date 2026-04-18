#pragma once

#include <Runtime/Data/Definitions/Definitions.h>
#include <Runtime/Data/Definitions/BitwiseOperator.h>
#include <Runtime/Data/Containers/Math.h>
#include <Runtime/Graphics/RHI/Util/ImageFormat.h>

#include <vector>

namespace Horizon
{
	class GfxImageView;

	enum class RenderingFlags : u32
	{
		None = 0,
		ContentsSecondary = 1 << 0,
		Suspending = 1 << 1,
		Resuming = 1 << 2
	};

	struct ClearValue
	{
		Math::Vec4f color = { 0.f, 0.f, 0.f, 1.f };
		f32 depth = 1.f;
		u32 stencil = 0;

		ClearValue& setColor(Math::Vec4f c) { color = c; return *this; }
		ClearValue& setDepth(f32 d) { depth = d; return *this; }
		ClearValue& setStencil(u32 s) { stencil = s; return *this; }
	};

	struct RenderingAttachmentInfo
	{
		GfxImageView* imageView = nullptr;
		ImageLayout imageLayout = ImageLayout::Undefined;
		AttachmentLoad loadOp = AttachmentLoad::DontCare;
		AttachmentStore storeOp = AttachmentStore::DontCare;
		ClearValue clearValue;

		RenderingAttachmentInfo& setImageView(GfxImageView* v) { imageView = v; return *this; }
		RenderingAttachmentInfo& setImageLayout(ImageLayout l) { imageLayout = l; return *this; }
		RenderingAttachmentInfo& setLoadOp(AttachmentLoad op) { loadOp = op; return *this; }
		RenderingAttachmentInfo& setStoreOp(AttachmentStore op) { storeOp = op; return *this; }
		RenderingAttachmentInfo& setClearValue(const ClearValue& cv) { clearValue = cv; return *this; }
	};

	struct RenderingInfo
	{
		RenderingFlags flags = RenderingFlags::None;
		Math::Vec2i renderAreaOffset = { 0, 0 };
		Math::Vec2u renderAreaExtent = { 0, 0 };
		u32 layerCount = 1;
		u32 viewMask = 0;
		std::vector<RenderingAttachmentInfo> colorAttachments;
		const RenderingAttachmentInfo* depthAttachment = nullptr;
		const RenderingAttachmentInfo* stencilAttachment = nullptr;

		RenderingInfo& setFlags(RenderingFlags f) { flags = f; return *this; }
		RenderingInfo& setRenderAreaOffset(Math::Vec2i offset) { renderAreaOffset = offset; return *this; }
		RenderingInfo& setRenderAreaExtent(Math::Vec2u extent) { renderAreaExtent = extent; return *this; }
		RenderingInfo& setLayerCount(u32 count) { layerCount = count; return *this; }
		RenderingInfo& setViewMask(u32 mask) { viewMask = mask; return *this; }
		RenderingInfo& addColorAttachment(const RenderingAttachmentInfo& attachment) { colorAttachments.push_back(attachment); return *this; }
		RenderingInfo& setDepthAttachment(const RenderingAttachmentInfo* attachment) { depthAttachment = attachment; return *this; }
		RenderingInfo& setStencilAttachment(const RenderingAttachmentInfo* attachment) { stencilAttachment = attachment; return *this; }
	};
}
