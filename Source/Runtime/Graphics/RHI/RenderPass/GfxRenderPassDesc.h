#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/PipelineFormat.h>

#include <vector>

namespace Horizon
{
    enum class AttachmentType : u8
    {
        Color,
        DepthStencil,
        Input
    };

    struct AttachmentDesc
    {
        ImageFormat format;
        AttachmentType type = AttachmentType::Color;
        ImageLayout initialLayout = ImageLayout::Undefined;
        ImageLayout finalLayout = ImageLayout::PresentSrcKHR;
        AttachmentLoad loadOp = AttachmentLoad::Clear;
        AttachmentStore storeOp = AttachmentStore::Store;

        AttachmentDesc& setFormat(ImageFormat fmt) { format = fmt; return *this; }
        AttachmentDesc& setType(AttachmentType t) { type = t; return *this; }
        AttachmentDesc& setInitialLayout(ImageLayout layout) { initialLayout = layout; return *this; }
        AttachmentDesc& setFinalLayout(ImageLayout layout) { finalLayout = layout; return *this; }
        AttachmentDesc& setLoadOp(AttachmentLoad op) { loadOp = op; return *this; }
        AttachmentDesc& setStoreOp(AttachmentStore op) { storeOp = op; return *this; }
    };

    struct SubpassDesc
    {
        std::vector<u32> colorAttachmentIndices;
        u32 depthAttachmentIndex = u32_max;
        std::vector<u32> inputAttachmentIndices;
        PipelineType bindPoint;

        SubpassDesc& addColorAttachment(u32 index) { colorAttachmentIndices.push_back(index); return *this; }
        SubpassDesc& setDepthAttachment(u32 index) { depthAttachmentIndex = index; return *this; }
        SubpassDesc& addInputAttachment(u32 index) { inputAttachmentIndices.push_back(index); return *this; }
        SubpassDesc& setBindPoint(PipelineType type) { bindPoint = type; return *this; }
    };

    struct GfxRenderPassDesc
    {
        std::vector<AttachmentDesc> attachments;
        std::vector<SubpassDesc> subpasses;

        GfxRenderPassDesc& addAttachment(const AttachmentDesc& attachment) { attachments.push_back(attachment); return *this; }
        GfxRenderPassDesc& addSubpass(const SubpassDesc& subpass) { subpasses.push_back(subpass); return *this; }
    };
}
