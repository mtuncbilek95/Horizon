#pragma once

#include <Runtime/Graphics/RHI/Util/CommandFormat.h>
#include <Runtime/Graphics/RHI/Util/ImageFormat.h>

namespace Horizon
{
    class GfxBuffer;
    class GfxCommandPool;
    class GfxRenderPass;
    class GfxFramebuffer;
    class GfxImage;

    struct CopyBufferDesc
    {
        GfxBuffer* src = nullptr;
        GfxBuffer* dst = nullptr;
        u32 srcOffset = 0;
        u32 dstOffset = 0;
        u32 size = 0;

        CopyBufferDesc& setSrc(GfxBuffer* s) { src = s; return *this; }
        CopyBufferDesc& setDst(GfxBuffer* d) { dst = d; return *this; }
        CopyBufferDesc& setSrcOffset(u32 offset) { srcOffset = offset; return *this; }
        CopyBufferDesc& setDstOffset(u32 offset) { dstOffset = offset; return *this; }
        CopyBufferDesc& setSize(u32 s) { size = s; return *this; }
    };

    struct BeginRenderDesc
    {
        GfxRenderPass* renderPass = nullptr;
        GfxFramebuffer* framebuffer = nullptr;
        u32 frameIndex = 0;
        Math::Vec2u renderArea = {};
        SubpassContent content = SubpassContent::ContentsInline;
        Math::Vec4f colorVal = { .2f, .2f, .2f, 1.f };
        Math::Vec2f depthVal = { 1.f, 0.f };
        b8 clearColor = false;
        b8 clearDepth = false;

        BeginRenderDesc& setRenderPass(GfxRenderPass* pass) { renderPass = pass; return *this; }
        BeginRenderDesc& setFramebuffer(GfxFramebuffer* fb) { framebuffer = fb; return *this; }
        BeginRenderDesc& setFrameIndex(u32 index) { frameIndex = index; return *this; }
        BeginRenderDesc& setRenderArea(Math::Vec2u area) { renderArea = area; return *this; }
        BeginRenderDesc& setContent(SubpassContent c) { content = c; return *this; }
        BeginRenderDesc& setColor(Math::Vec4f color) { colorVal = color; return *this; }
        BeginRenderDesc& setDepth(Math::Vec2f depth) { depthVal = depth; return *this; }
        BeginRenderDesc& setClearColor(b8 color) { clearColor = color; return *this; }
        BeginRenderDesc& setClearDepth(b8 depth) { clearDepth = depth; return *this; }

    };

    struct InheritanceDesc
    {
        GfxRenderPass* renderPass = nullptr;
        GfxFramebuffer* framebuffer = nullptr;
        u32 subpass = 0;
        u32 frameIndex = 0;

        InheritanceDesc& setRenderPass(GfxRenderPass* pass) { renderPass = pass; return *this; }
        InheritanceDesc& setFramebuffer(GfxFramebuffer* fb) { framebuffer = fb; return *this; }
        InheritanceDesc& setFrameIndex(u32 index) { frameIndex = index; return *this; }
    };

	struct DynamicInheritanceDesc
	{
		std::vector<ImageFormat> colorFormats;
		ImageFormat depthFormat = ImageFormat::Undefined;

		DynamicInheritanceDesc& addColorFormat(ImageFormat format) { colorFormats.push_back(format); return *this; }
		DynamicInheritanceDesc& setDepthFormat(ImageFormat format) { depthFormat = format; return *this; }
	};

    struct BlitImageDesc
    {
        GfxImage*   srcImage   = nullptr;
        GfxImage*   dstImage   = nullptr;
        Math::Vec3u srcSize    = {};
        Math::Vec3u dstSize    = {};
        ImageLayout srcLayout  = ImageLayout::TransferSrcOptimal;
        ImageLayout dstLayout  = ImageLayout::TransferDstOptimal;

        BlitImageDesc& setSrcImage(GfxImage* img)         { srcImage  = img; return *this; }
        BlitImageDesc& setDstImage(GfxImage* img)         { dstImage  = img; return *this; }
        BlitImageDesc& setSrcSize(const Math::Vec3u& s)   { srcSize   = s;   return *this; }
        BlitImageDesc& setDstSize(const Math::Vec3u& s)   { dstSize   = s;   return *this; }
        BlitImageDesc& setSrcLayout(ImageLayout l)        { srcLayout = l;   return *this; }
        BlitImageDesc& setDstLayout(ImageLayout l)        { dstLayout = l;   return *this; }
    };

    // Simplified image memory barrier — access masks are deduced from the layout transition.
    struct ImageBarrierDesc
    {
        GfxImage*   image     = nullptr;
        ImageLayout oldLayout = ImageLayout::Undefined;
        ImageLayout newLayout = ImageLayout::General;
        ImageAspect aspect    = ImageAspect::Color;

        ImageBarrierDesc& setImage(GfxImage* img)       { image     = img; return *this; }
        ImageBarrierDesc& setOldLayout(ImageLayout l)   { oldLayout = l;   return *this; }
        ImageBarrierDesc& setNewLayout(ImageLayout l)   { newLayout = l;   return *this; }
        ImageBarrierDesc& setAspect(ImageAspect a)      { aspect    = a;   return *this; }
    };

    struct GfxCommandBufferDesc
    {
        GfxCommandPool* pool;
        CommandLevel level;

        GfxCommandBufferDesc& setPool(GfxCommandPool* pol) { pool = pol; return *this; }
        GfxCommandBufferDesc& setLevel(CommandLevel lvl) { level = lvl; return *this; }
    };
}
