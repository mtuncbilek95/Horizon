#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandBufferDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Util/RenderingInfo.h>
#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

#include <vector>

namespace Horizon
{
    class GfxBuffer;
    class GfxDescriptorSet;
    class GfxDescriptorBuffer;
    class GfxImage;
    class GfxPipeline;
    class GfxCommandPool;

    class GfxCommandBuffer : public GfxObject
    {
    public:
        GfxCommandBuffer(const GfxCommandBufferDesc& desc, GfxDevice* pDevice);
        virtual ~GfxCommandBuffer() override = default;

        virtual void* Buffer() const = 0;
        GfxCommandPool* Pool() const { return m_desc.pool; }

        virtual void BeginRecord(CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const = 0;
        virtual void BeginRecord(const InheritanceDesc& desc, CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const = 0;
        virtual void BeginRecord(const DynamicInheritanceDesc& desc, CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const = 0;
        virtual void EndRecord() const = 0;

		virtual void BeginRendering(const RenderingInfo& info) = 0;
		virtual void EndRendering() = 0;

        virtual void BeginRenderPass(const BeginRenderDesc& desc) const = 0;
        virtual void EndRenderPass() const = 0;

        virtual void BindPipeline(GfxPipeline* pipeline) const;
        virtual void BindPushConstants(ShaderStage stage, u32 offset, u32 size, const void* data) const = 0;
		virtual void BindViewport(const Math::Vec2f& xy, const Math::Vec2f& size, const Math::Vec2f& depth) const = 0;
		virtual void BindScissor(const Math::Vec2i& offset, const Math::Vec2u& extent) const = 0;

        virtual void BindVertices(const std::vector<GfxBuffer*> buffers) const = 0;
        virtual void BindIndex(const GfxBuffer* buffer, usize offset) const = 0;

        virtual void DrawVertexed(u32 vertexCount, u32 firstVertex, u32 firstInstance, u32 instanceCount) const = 0;
        virtual void DrawIndexed(u32 indexCount, u32 indexOffset, u32 vertexOffset, u32 instanceOffset, u32 instanceCount) const = 0;
        virtual void DrawMeshTask(u32 groupCountX, u32 groupCountY, u32 groupCountZ) const = 0;
        virtual void ExecuteCommands(const std::vector<GfxCommandBuffer*> buffers) const = 0;

        virtual void DispatchCompute(u32 x, u32 y, u32 z) const = 0;
        virtual void BindDescriptorBuffer(const GfxPipeline* pipeline, ShaderStage stage, u32 binding, GfxDescriptorBuffer* buffer) const = 0;
        virtual void BindDescriptorSet(const GfxPipeline* pipeline, const GfxDescriptorSet* set, u32 firstSet) const = 0;

        virtual void CopyStageToBuffer(const CopyBufferDesc& desc) const = 0;
        virtual void BlitImage(const BlitImageDesc& desc) const = 0;
        virtual void ImageBarrier(const ImageBarrierDesc& desc) const = 0;

    protected:
        mutable GfxPipeline* m_boundPipeline = nullptr;

    private:
        GfxCommandBufferDesc m_desc;
    };
}
