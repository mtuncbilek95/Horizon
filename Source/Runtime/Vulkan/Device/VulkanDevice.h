#pragma once

#include <Runtime/RHI/Device/Device.h>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <Runtime/Vulkan/Device/VulkanDeviceFeatures.h>
#include <Runtime/Vulkan/Device/VulkanInstanceFeatures.h>

namespace Horizon::RHI 
{
	class VulkanDevice final : public Device 
	{
	public:
		VulkanDevice();
		~VulkanDevice();

		b8 Initialize(const DeviceDesc& desc);
		void CloseAll();

		UniquePtr<Buffer> CreateRHIBuffer(const BufferDesc& desc) final;
		UniquePtr<Texture> CreateRHITexture(const TextureDesc& desc) final;
		UniquePtr<TextureView> CreateRHITextureView(const TextureViewDesc& desc) final;
		UniquePtr<Sampler> CreateRHISampler(const SamplerDesc& desc) final;
		UniquePtr<ShaderModule> CreateRHIShaderModule(const ShaderModuleDesc& desc) final;
		UniquePtr<PipelineLayout> CreateRHIPipelineLayout(const PipelineLayoutDesc& desc) final;
		UniquePtr<GraphicsPipeline> CreateRHIPipeline(const GraphicsPipelineDesc& desc) final;
		UniquePtr<ComputePipeline> CreateRHIPipeline(const ComputePipelineDesc& desc) final;
		UniquePtr<DescriptorHeap> CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc) final;
		UniquePtr<DescriptorTable> CreateRHIDescriptorTable(const DescriptorTableDesc& desc) final;
		UniquePtr<CommandPool> CreateRHICommandPool(const CommandPoolDesc& desc) final;
		UniquePtr<CommandList> CreateRHICommandList(const CommandListDesc& desc) final;
		UniquePtr<Fence> CreateRHIFence(const FenceDesc& desc) final;
		UniquePtr<Swapchain> CreateRHISwapchain(const SwapchainDesc& desc) final;

		void Submit(QueueType type, ReadOnlyList<CommandList*> lists) final;
		u64 Signal(QueueType type, Fence* pFence) final;
		void Wait(QueueType type, Fence* pFence, u64 value) final;
		void WaitIdle() final;

		u64 GetTimestampFrequency(QueueType type) final { return 0; }

		void InitializeImGui(u32 framesInFlight, DescriptorHeap* pResourceHeap, TextureFormat colorFormat) final;
		u64 RegisterImGuiTexture(TextureView* pView, Sampler* pSampler) final;
		void UnregisterImGuiTexture(u64 textureId) final;
		void NewFrameImGui() final;
		void ShutdownImGui() final;

		VkInstance GetInstance() const { return m_instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
		VkDevice GetDevice() const { return m_device; }
		VkQueue GetQueue(QueueType type) const { return m_queues[static_cast<u32>(type)]; }
		u32 GetQueueFamily(QueueType type) const { return m_queueFamilies[static_cast<u32>(type)]; }
		VmaAllocator GetAllocator() const { return m_allocator; }

		const DeviceDesc& GetDesc() const { return m_deviceDesc; }
	private:
		b8 CreateInstance();
		b8 CreateDebugMessenger();
		b8 PickPhysicalDevice();
		b8 CreateDevice();
		b8 CreateVMAAllocator();
		
		b8 QueryInstanceFeatures();
		b8 CheckDeviceSupport(VkPhysicalDevice physicalDevice);
		b8 QueryDeviceFeatures();
		List<VkDeviceQueueCreateInfo> PickQueueFamilies();
		List<const c8*> GetDeviceExtensions();
		List<const c8*> GetInstanceExtensions();
		List<const c8*> GetInstanceLayers();
		List<const c8*> GetDeviceExtensions(VkPhysicalDevice device);
		VkPhysicalDeviceFeatures2 GetDeviceFeaturesChain(VkPhysicalDevice device);
	private:
		VkInstance m_instance{};
		VkDebugUtilsMessengerEXT m_debugMessenger{};
		VkPhysicalDevice m_physicalDevice{};
		VkDevice m_device{};
		VkQueue m_queues[static_cast<u32>(QueueType::Count)]{};
		u32 m_queueFamilies[static_cast<u32>(QueueType::Count)]{};
		VmaAllocator m_allocator{};

		DeviceDesc m_deviceDesc{};
		VulkanInstanceFeatures m_instanceFeatures{};
		VulkanDeviceFeatures m_deviceFeatures{};

		u32 m_resourceDescriptorSize = 0;
		u32 m_samplerDescriptorSize = 0;

		u32 m_resourceHeapReservedRange = 0;
		u32 m_samplerHeapReservedRange = 0;
	};
}