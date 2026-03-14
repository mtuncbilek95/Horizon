#pragma once

#include <Runtime/Graphics/RHI/Buffer/GfxBufferDesc.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBufferDesc.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPoolDesc.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayoutDesc.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorPoolDesc.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorSetDesc.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBufferDesc.h>
#include <Runtime/Graphics/RHI/Device/GfxDeviceDesc.h>
#include <Runtime/Graphics/RHI/Framebuffer/GfxFramebufferDesc.h>
#include <Runtime/Graphics/RHI/Image/GfxImageDesc.h>
#include <Runtime/Graphics/RHI/Image/GfxImageViewDesc.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipelineDesc.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueueDesc.h>
#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPassDesc.h>
#include <Runtime/Graphics/RHI/Sampler/GfxSamplerDesc.h>
#include <Runtime/Graphics/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchainDesc.h>
#include <Runtime/Graphics/RHI/Sync/GfxFenceDesc.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphoreDesc.h>

#include <memory>

namespace Horizon
{
	class GfxInstance;
	class GfxQueue;
	class GfxSwapchain;
	class GfxImage;
	class GfxImageView;
	class GfxBuffer;
	class GfxPipeline;
	class GfxFence;
	class GfxSemaphore;
	class GfxDescriptorLayout;
	class GfxDescriptorPool;
	class GfxDescriptorSet;
	class GfxDescriptorBuffer;
	class GfxSampler;
	class GfxShader;
	class GfxCommandPool;
	class GfxCommandBuffer;
	class GfxFramebuffer;
	class GfxRenderPass;

	/**
	 * @class GfxDevice
	 * @brief Main device that controls every creation.
	 */
	class GfxDevice
	{
	public:
		GfxDevice(const GfxDeviceDesc& desc, GfxInstance* pInstance);
		virtual ~GfxDevice() = default;

		virtual void* Device() const = 0;
		virtual void* Instance() const = 0;
		virtual void* Adapter() const = 0;
		virtual void* Allocator() const = 0;

		virtual void WaitIdle() const = 0;

		virtual std::shared_ptr<GfxQueue> CreateQueue(QueueType type) = 0;
		virtual std::shared_ptr<GfxImage> CreateImage(const GfxImageDesc& desc) = 0;
		virtual std::shared_ptr<GfxSwapchain> CreateSwapchain(const GfxSwapchainDesc& desc) = 0;
		virtual std::shared_ptr<GfxImageView> CreateView(const GfxImageViewDesc& desc) = 0;
		virtual std::shared_ptr<GfxBuffer> CreateBuffer(const GfxBufferDesc& desc) = 0;
		virtual std::shared_ptr<GfxPipeline> CreateGraphicsPipeline(const GfxGraphicsPipelineDesc& desc) = 0;
		virtual std::shared_ptr<GfxPipeline> CreateComputePipeline(const GfxComputePipelineDesc& desc) = 0;
		virtual std::shared_ptr<GfxFence> CreateSyncFence(const GfxFenceDesc& desc) = 0;
		virtual std::shared_ptr<GfxSemaphore> CreateSyncSemaphore(const GfxSemaphoreDesc& desc) = 0;
		virtual std::shared_ptr<GfxDescriptorLayout> CreateDescriptorLayout(const GfxDescriptorLayoutDesc& desc) = 0;
		virtual std::shared_ptr<GfxDescriptorPool> CreateDescriptorPool(const GfxDescriptorPoolDesc& desc) = 0;
		virtual std::shared_ptr<GfxDescriptorSet> CreateDescriptorSet(const GfxDescriptorSetDesc& desc) = 0;
		virtual std::shared_ptr<GfxDescriptorBuffer> CreateDescriptorBuffer(const GfxDescriptorBufferDesc& desc) = 0;
		virtual std::shared_ptr<GfxSampler> CreateSampler(const GfxSamplerDesc& desc) = 0;
		virtual std::shared_ptr<GfxShader> CreateShader(const GfxShaderDesc& desc) = 0;
		virtual std::shared_ptr<GfxCommandPool> CreateCommandPool(const GfxCommandPoolDesc& desc) = 0;
		virtual std::shared_ptr<GfxCommandBuffer> CreateCommandBuffer(const GfxCommandBufferDesc& desc) = 0;
		virtual std::shared_ptr<GfxFramebuffer> CreateFramebuffer(const GfxFramebufferDesc& desc) = 0;
		virtual std::shared_ptr<GfxRenderPass> CreateRenderPass(const GfxRenderPassDesc& desc) = 0;

	private:
		GfxDeviceDesc m_desc;
	};
}
