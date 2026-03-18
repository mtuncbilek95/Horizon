#pragma once

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include <memory>
#include <vector>

namespace Horizon
{
    class GfxVkDevice final : public GfxDevice
    {
        struct QueueFamily
        {
            QueueFamily() : m_familyIndex(255), m_queueCount(0), m_requestedCount(0) {}
            ~QueueFamily() {}

            VkQueue GetFreeQueue();
            void FillQueues(VkDevice device);

            u32 m_familyIndex;
            u32 m_queueCount;
            u32 m_requestedCount;
            std::vector<VkQueue> m_queues;
        };

    public:
        GfxVkDevice(const GfxDeviceDesc& desc, GfxInstance* pInstance);
        ~GfxVkDevice() override;

        void* Device() const final;
        void* Instance() const final;
        void* Adapter() const final;
        void* Allocator() const final;

        void WaitIdle() const final;
        
        std::shared_ptr<GfxQueue> CreateQueue(QueueType type) final;
        std::shared_ptr<GfxImage> CreateImage(const GfxImageDesc& desc) final;
        std::shared_ptr<GfxImageView> CreateView(const GfxImageViewDesc& desc) final;
        std::shared_ptr<GfxSwapchain> CreateSwapchain(const GfxSwapchainDesc& desc) final;
        std::shared_ptr<GfxBuffer> CreateBuffer(const GfxBufferDesc& desc) final;
        std::shared_ptr<GfxPipeline> CreateGraphicsPipeline(const GfxGraphicsPipelineDesc& desc) final;
        std::shared_ptr<GfxPipeline> CreateComputePipeline(const GfxComputePipelineDesc& desc) final;
        std::shared_ptr<GfxFence> CreateSyncFence(const GfxFenceDesc& desc) final;
        std::shared_ptr<GfxSemaphore> CreateSyncSemaphore(const GfxSemaphoreDesc& desc) final;
        std::shared_ptr<GfxDescriptorLayout> CreateDescriptorLayout(const GfxDescriptorLayoutDesc& desc) final;
        std::shared_ptr<GfxDescriptorPool> CreateDescriptorPool(const GfxDescriptorPoolDesc& desc) final;
        std::shared_ptr<GfxDescriptorSet> CreateDescriptorSet(const GfxDescriptorSetDesc& desc) final;
        std::shared_ptr<GfxDescriptorBuffer> CreateDescriptorBuffer(const GfxDescriptorBufferDesc& desc) final;
        std::shared_ptr<GfxSampler> CreateSampler(const GfxSamplerDesc& desc) final;
        std::shared_ptr<GfxShader> CreateShader(const GfxShaderDesc& desc) final;
        std::shared_ptr<GfxCommandPool> CreateCommandPool(const GfxCommandPoolDesc& desc) final;
        std::shared_ptr<GfxCommandBuffer> CreateCommandBuffer(const GfxCommandBufferDesc& desc) final;
        std::shared_ptr<GfxFramebuffer> CreateFramebuffer(const GfxFramebufferDesc& desc) final;
        std::shared_ptr<GfxRenderPass> CreateRenderPass(const GfxRenderPassDesc& desc) final;

        usize DescTypeSize(DescriptorType type) const;
        usize DescBufferAlignment() const { return m_descBufferProps.descriptorBufferOffsetAlignment; }
        
    private:
        VkDevice m_device;
        VkInstance m_instance;
        VkPhysicalDevice m_adapter;
        VmaAllocator m_allocator;

        QueueFamily m_graphicsQueue;
        QueueFamily m_computeQueue;
        QueueFamily m_transferQueue;

        VkPhysicalDeviceProperties2 m_deviceProps;
        VkPhysicalDeviceDescriptorBufferPropertiesEXT m_descBufferProps;
        VkPhysicalDeviceMeshShaderPropertiesEXT m_meshShaderProps;
        VkPhysicalDeviceSubgroupProperties m_subgroupProps;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingProps;
    };
}
