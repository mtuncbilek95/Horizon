#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

#include <vector>

namespace Horizon
{
    class GfxVkCommandBuffer final : public GfxCommandBuffer
    {
    public:
        GfxVkCommandBuffer(const GfxCommandBufferDesc& desc, GfxDevice* pDevice);
        ~GfxVkCommandBuffer() override;

        void* Buffer() const final;

        void BeginRecord(CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const final;
        void BeginRecord(const InheritanceDesc& desc, CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const final;
		void BeginRecord(const DynamicInheritanceDesc& desc, CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit) const final;
        void EndRecord() const final;

		void BeginRendering(const RenderingInfo& info) final;
		void EndRendering() final;

        void BeginRenderPass(const BeginRenderDesc& desc) const final;
        void EndRenderPass() const final;

        void BindPipeline(GfxPipeline* pipeline) const final;
        void BindPushConstants(ShaderStage stage, u32 offset, u32 size, const void* data) const final;
		void BindViewport(const Math::Vec2f& xy, const Math::Vec2f& size, const Math::Vec2f& depth) const final;
		void BindScissor(const Math::Vec2i& offset, const Math::Vec2u& extent) const final;

        void BindVertices(const std::vector<GfxBuffer*> buffers) const final;
        void BindIndex(const GfxBuffer* buffer, usize offset) const final;

        void DrawVertexed(u32 vertexCount, u32 firstVertex, u32 firstInstance, u32 instanceCount) const final;
        void DrawIndexed(u32 indexCount, u32 indexOffset, u32 vertexOffset, u32 instanceOffset, u32 instanceCount) const final;
        void ExecuteCommands(const std::vector<GfxCommandBuffer*> buffers) const final;

        void DrawMeshTask(u32 groupCountX, u32 groupCountY, u32 groupCountZ) const final;

        void DispatchCompute(u32 x, u32 y, u32 z) const final;
        void BindDescriptorBuffer(const GfxPipeline* pipeline, ShaderStage stage, u32 binding, GfxDescriptorBuffer* buffer) const final;
        void BindDescriptorSet(const GfxPipeline* pipeline, const GfxDescriptorSet* set, u32 firstSet) const final;

        void CopyStageToBuffer(const CopyBufferDesc& desc) const final;
        void BlitImage(const BlitImageDesc& desc) const final;
        void ImageBarrier(const ImageBarrierDesc& desc) const final;

    private:
        VkCommandBuffer m_buffer;
    };
}
