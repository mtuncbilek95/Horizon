#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/PipelineFormat.h>
#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <vector>

namespace Horizon
{
    class GfxDescriptorLayout;
    class GfxShader;
    class GfxRenderPass;

    struct Viewport
    {
        Math::Vec2f position = { 0.f, 0.f };
        Math::Vec2f size = { 0.f, 0.f };
        Math::Vec2f depth = { 0.f, 1.f };

        Viewport& setPosition(const Math::Vec2f pos) { position = pos; return *this; }
        Viewport& setSize(const Math::Vec2f sz) { size = sz; return *this; }
        Viewport& setDepth(const Math::Vec2f dp) { depth = dp; return *this; }
    };

    struct Scissor
    {
        Math::Vec2i offset = { 0, 0 };
        Math::Vec2u extent = { 0, 0 };

        Scissor& setOffset(const Math::Vec2i ofs) { offset = ofs; return *this; }
        Scissor& setExtent(const Math::Vec2u ext) { extent = ext; return *this; }
    };

    struct VertexInputBinding
    {
        u32 binding;
        u32 stride;
        VertexInputRate inputRate = VertexInputRate::Vertex;

        VertexInputBinding& setBinding(u32 bnd) { binding = bnd; return *this; }
        VertexInputBinding& setStride(u32 str) { stride = str; return *this; }
        VertexInputBinding& setBinding(VertexInputRate rate) { inputRate = rate; return *this; }
    };

    struct VertexInputAttribute
    {
        u32 location;
        u32 binding;
        ImageFormat format;
        u32 offset;

        VertexInputAttribute& setLocation(u32 loc) { location = loc; return *this; }
        VertexInputAttribute& setBindng(u32 bnd) { binding = bnd; return *this; }
        VertexInputAttribute& setFormat(ImageFormat frmt) { format = frmt; return *this; }
        VertexInputAttribute& setOffset(u32 ofs) { offset = ofs; return *this; }
    };

    struct PushConstantRange
    {
        ShaderStage stage;
        u32 offset;
        u32 sizeOfData;

        PushConstantRange& setStage(ShaderStage stg) { stage = stg; return *this; }
        PushConstantRange& setOffset(u32 ofs) { offset = ofs; return *this; }
        PushConstantRange& setSizeOfData(u32 size) { sizeOfData = size; return *this; }
    };

    struct PushConstants
    {
        std::vector<PushConstantRange> ranges;

        PushConstants& addRange(const PushConstantRange& range) { ranges.push_back(range); return *this; }
    };

    struct InputBinding
    {
        VertexInputRate inputRate;
        std::vector<ImageFormat> attributes;

        InputBinding& setInputRate(VertexInputRate rate) { inputRate = rate; return *this; }
        InputBinding& addAttribute(ImageFormat format) { attributes.push_back(format); return *this; }
    };

    struct InputAssembler
    {
        PrimitiveTopology topology;
        b8 primitiveRestart;
        std::vector<InputBinding> bindings;

        InputAssembler& setTopology(PrimitiveTopology top) { topology = top; return *this; }
        InputAssembler& setPrimitiveRestart(b8 restart) { primitiveRestart = restart; return *this; }
        InputAssembler& addBinding(const InputBinding& binding) { bindings.push_back(binding); return *this; }
    };

    struct RasterizerState
    {
        b8 depthBiasEnable;
        f32 depthBiasConstantFactor;
        f32 depthBiasClamp;
        f32 depthBiasSlope;
        PolygonMode polygon;
        CullMode cull;
        FaceOrientation face;

        RasterizerState& setDepthBiasEnable(b8 enable) { depthBiasEnable = enable; return *this; }
        RasterizerState& setDepthBiasConstantFactor(f32 factor) { depthBiasConstantFactor = factor; return *this; }
        RasterizerState& setDepthBiasClamp(f32 clamp) { depthBiasClamp = clamp; return *this; }
        RasterizerState& setDepthBiasSlope(f32 slope) { depthBiasSlope = slope; return *this; }
        RasterizerState& setPolygonMode(PolygonMode mode) { polygon = mode; return *this; }
        RasterizerState& setCullMode(CullMode mode) { cull = mode; return *this; }
        RasterizerState& setFaceOrientation(FaceOrientation orientation) { face = orientation; return *this; }
    };

    struct BlendAttachment
    {
        b8 blendEnable;
        BlendFactor srcColor;
        BlendFactor dstColor;
        BlendOp colorOp;
        BlendFactor srcAlpha;
        BlendFactor dstAlpha;
        BlendOp alphaOp;
        ColorComponent colorMask;

        BlendAttachment& setBlendEnable(b8 enable) { blendEnable = enable; return *this; }
        BlendAttachment& setSrcColor(BlendFactor factor) { srcColor = factor; return *this; }
        BlendAttachment& setDstColor(BlendFactor factor) { dstColor = factor; return *this; }
        BlendAttachment& setColorOp(BlendOp op) { colorOp = op; return *this; }
        BlendAttachment& setSrcAlpha(BlendFactor factor) { srcAlpha = factor; return *this; }
        BlendAttachment& setDstAlpha(BlendFactor factor) { dstAlpha = factor; return *this; }
        BlendAttachment& setAlphaOp(BlendOp op) { alphaOp = op; return *this; }
        BlendAttachment& setColorMask(ColorComponent mask) { colorMask = mask; return *this; }
    };

    struct BlendState
    {
        b8 logicEnable;
        LogicOp logicOp;
        std::vector<BlendAttachment> attachments;

        BlendState& setLogicEnable(b8 enable) { logicEnable = enable; return *this; }
        BlendState& setLogicOp(LogicOp op) { logicOp = op; return *this; }
        BlendState& addAttachment(const BlendAttachment& attachment) { attachments.push_back(attachment); return *this; }
    };

    struct DepthStencilState
    {
        b8 depthTestEnable;
        b8 depthWriteEnable;
        CompareOp depthOp;
        b8 depthBoundEnable;
        f32 minDepth;
        f32 maxDepth;

        DepthStencilState& setDepthTestEnable(b8 enable) { depthTestEnable = enable; return *this; }
        DepthStencilState& setDepthWriteEnable(b8 enable) { depthWriteEnable = enable; return *this; }
        DepthStencilState& setDepthOp(CompareOp op) { depthOp = op; return *this; }
        DepthStencilState& setDepthBoundEnable(b8 enable) { depthBoundEnable = enable; return *this; }
        DepthStencilState& setMinDepth(f32 min) { minDepth = min; return *this; }
        DepthStencilState& setMaxDepth(f32 max) { maxDepth = max; return *this; }
    };

    struct DynamicRendering
    {
        std::vector<ImageFormat> colorAttachmentFormats;
        ImageFormat depthAttachmentFormat;
        ImageFormat stencilAttachmentFormat;

        DynamicRendering& addColorAttachmentFormat(ImageFormat format) { colorAttachmentFormats.push_back(format); return *this; }
        DynamicRendering& setDepthAttachmentFormat(ImageFormat format) { depthAttachmentFormat = format; return *this; }
        DynamicRendering& setStencilAttachmentFormat(ImageFormat format) { stencilAttachmentFormat = format; return *this; }
    };

    struct GfxGraphicsPipelineDesc 
    {
        std::vector<GfxDescriptorLayout*> layouts;
        std::vector<GfxShader*> shaders;
        InputAssembler input;
        RasterizerState rasterizer;
        PushConstants pushes;
        BlendState blend;
        DepthStencilState depthStencil;
        Viewport viewport;
        Scissor scissor;
        std::vector<DynamicState> dynamicStates;
        GfxRenderPass* pass;
        DynamicRendering dynamicRendering;
        PipelineFlags flags;

        GfxGraphicsPipelineDesc& addLayout(GfxDescriptorLayout* layout) { layouts.push_back(layout); return *this; }
        GfxGraphicsPipelineDesc& addShader(GfxShader* shader) { shaders.push_back(shader); return *this; }
        GfxGraphicsPipelineDesc& setInput(const InputAssembler& ia) { input = ia; return *this; }
        GfxGraphicsPipelineDesc& setRasterizer(const RasterizerState& rs) { rasterizer = rs; return *this; }
        GfxGraphicsPipelineDesc& setPushConstants(const PushConstants& pc) { pushes = pc; return *this; }
        GfxGraphicsPipelineDesc& setBlend(const BlendState& bs) { blend = bs; return *this; }
        GfxGraphicsPipelineDesc& setDepthStencil(const DepthStencilState& ds) { depthStencil = ds; return *this; }
        GfxGraphicsPipelineDesc& setViewport(const Viewport& vp) { viewport = vp; return *this; }
        GfxGraphicsPipelineDesc& setScissor(const Scissor& sc) { scissor = sc; return *this; }
        GfxGraphicsPipelineDesc& addDynamicState(DynamicState state) { dynamicStates.push_back(state); return *this; }
        GfxGraphicsPipelineDesc& setRenderPass(GfxRenderPass* rp) { pass = rp; return *this; }
        GfxGraphicsPipelineDesc& setDynamicRendering(const DynamicRendering& dr) { dynamicRendering = dr; return *this; }
        GfxGraphicsPipelineDesc& setFlags(PipelineFlags f) { flags = f; return *this; }
    };


    struct GfxComputePipelineDesc
    {
        std::vector<GfxDescriptorLayout*> layouts;
        GfxShader*    shader = nullptr;
        PushConstants pushes;

        GfxComputePipelineDesc& addLayout(GfxDescriptorLayout* layout)    { layouts.push_back(layout); return *this; }
        GfxComputePipelineDesc& setShader(GfxShader* s)                   { shader = s;  return *this; }
        GfxComputePipelineDesc& setPushConstants(const PushConstants& pc) { pushes = pc; return *this; }
    };

    struct GfxRaytracingPipelineDesc
    {
    };
}
