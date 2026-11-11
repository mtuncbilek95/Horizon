#include <Runtime/Vulkan/Device/VulkanDevice.h>

#include <Runtime/Vulkan/Buffer/VulkanBuffer.h>
#include <Runtime/Vulkan/Command/VulkanCommandList.h>
#include <Runtime/Vulkan/Command/VulkanCommandPool.h>
#include <Runtime/Vulkan/Helpers/VulkanHelpers.h>
#include <Runtime/Vulkan/Descriptor/VulkanDescriptorHeap.h>
#include <Runtime/Vulkan/Descriptor/VulkanDescriptorTable.h>
#include <Runtime/Vulkan/Fence/VulkanFence.h>
#include <Runtime/Vulkan/Pipeline/VulkanComputePipeline.h>
#include <Runtime/Vulkan/Pipeline/VulkanGraphicsPipeline.h>
#include <Runtime/Vulkan/Pipeline/VulkanPipelineLayout.h>
#include <Runtime/Vulkan/Sampler/VulkanSampler.h>
#include <Runtime/Vulkan/Shader/VulkanShaderModule.h>
#include <Runtime/Vulkan/Swapchain/VulkanSwapchain.h>
#include <Runtime/Vulkan/Texture/VulkanTexture.h>
#include <Runtime/Vulkan/Texture/VulkanTextureView.h>
#include <Runtime/Vulkan/Helpers/VulkanHelpers.h>
#include <Runtime/Log/Terminal.h>
#include "imgui_impl_vulkan.h"
#include "vulkan/vulkan_core.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace Horizon::RHI {
	VkBool32 VKAPI_PTR DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			Terminal::Error("Vulkan", "{}", pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
			Terminal::Warn("Vulkan", "{}", pCallbackData->pMessage);
		}
		else {
			Terminal::Info("Vulkan", "{}", pCallbackData->pMessage);
		}
		return VK_FALSE;
	}

	b8 VulkanDevice::QueryInstanceFeatures()
	{
		u32 apiVersion{};
		vkEnumerateInstanceVersion(&apiVersion);

		if (apiVersion < VK_API_VERSION_1_4)
			return false;

		if (m_deviceDesc.enableDebug) {
			u32 layerCount{};
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
			List<VkLayerProperties> layers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, layers.GetData());

			b8 validationLayerSupport{};
			for (const auto &layer : layers)
			{
				if (strcmp("VK_LAYER_KHRONOS_validation", layer.layerName) == 0)
				{
					validationLayerSupport = true;
					break;
				}
			}

			if (!validationLayerSupport)
				return false;
		}

		{
			u32 extensionCount{};
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			List<VkExtensionProperties> extensions(extensionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.GetData());

			b8 surfaceSupport{};
			b8 debugUtilsSupport{};
			b8 hasSurface{};
			for (const auto& extension : extensions)
			{
				if (!surfaceSupport && strcmp(VK_KHR_SURFACE_EXTENSION_NAME, extension.extensionName) == 0)
					surfaceSupport = true;

				else if (m_deviceDesc.enableDebug && !debugUtilsSupport && strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, extension.extensionName) == 0)
					debugUtilsSupport = true;
			}

			if (!surfaceSupport)
				return false;

			if (m_deviceDesc.enableDebug && !debugUtilsSupport)
				return false;
		}

		return true;
	}

	List<const c8*> VulkanDevice::GetDeviceExtensions()
	{
		static List<const c8*> extensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
			VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME,
			VK_EXT_MESH_SHADER_EXTENSION_NAME,
			VK_KHR_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME,
			// sm6.6 contains this shit
			VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME
		};

		return extensions;
	}

	List<const c8*> VulkanDevice::GetInstanceExtensions()
	{
		List<const c8*> out{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            #ifdef HORIZON_WINDOWS
                "VK_KHR_win32_surface"

			#elifdef HORIZON_LINUX
			    "VK_KHR_xcb_surface"

			#elifdef HORIZON_ANDROID
			    "VK_KHR_android_surface"
			#else
			    #error unsupported_os
			#endif
		};

		if (m_deviceDesc.enableDebug)
		{
			out.EmplaceBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			out.EmplaceBack(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
		}

		return out;
	}

	List<const c8*> VulkanDevice::GetInstanceLayers()
	{
		List<const c8*> out;

		if (m_deviceDesc.enableDebug)
			out.EmplaceBack("VK_LAYER_KHRONOS_validation");

		return out;
	}

	// TODO: check other stuff like features and properties
	b8 VulkanDevice::CheckDeviceSupport(VkPhysicalDevice physicalDevice)
	{
		u32 extensionCount{};
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		List<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.GetData());

		const List<const c8*> required_extensions = GetDeviceExtensions();

		b8 deviceSupported{true};

		for (const auto &required_extension : required_extensions) {
			b8 extSupported{false};

			for (const auto &extension : extensions) {
				if (strcmp(required_extension, extension.extensionName) == 0) {
					extSupported = true;
					break;
				}
			}

			if (!extSupported) {
				Terminal::Info(StringHelper::GetTypeName(this), "Device dosen't support {} extension", required_extension);
				deviceSupported = false;
			}
		}

		Terminal::Info(StringHelper::GetTypeName(this), "Device supported");
		return deviceSupported;
	}

	// UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc)
	// {
	// 	UniquePtr<VulkanDevice> pDevice = MakeUnique<VulkanDevice>();
	// 	if (pDevice->Initialize(desc))
	// 		return pDevice;

	// 	return nullptr;
	// }

	VulkanDevice::VulkanDevice()
	{
	}
	VulkanDevice::~VulkanDevice()
	{
		CloseAll();
	}

	// TODO(0x): i forgot errors
	b8 VulkanDevice::Initialize(const DeviceDesc& desc)
	{
		m_deviceDesc = desc;

		// load global funcs
		if (const auto res = volkInitialize(); res != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to initialize volk, VkResult({})", (i32)res);
			return false;
		}

		if (!QueryInstanceFeatures())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to vulkan instance not dosen't support required features");
			return false;
		}

		if (!CreateInstance()) {
			CloseAll();
			return false;
		}

		volkLoadInstance(m_instance);

		if (m_deviceDesc.enableDebug) {
			if (!CreateDebugMessenger()) {
				CloseAll();
				return false;
			}
		}

		if (!PickPhysicalDevice()) {
			Terminal::Error(StringHelper::GetTypeName(this), "There is no physical device that supports the required features");
			CloseAll();
			return false;
		}

		{
			VkPhysicalDeviceProperties deviceProperties{};
			vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

			Terminal::Info(StringHelper::GetTypeName(this), "Picked device: {}", deviceProperties.deviceName);
		}

		if (!CreateDevice()) {
			CloseAll();
			return false;
		}

		volkLoadDevice(m_device);

		if (!CreateVMAAllocator()) {
			CloseAll();
			return false;
		}

		return true;
	}

	void VulkanDevice::CloseAll()
	{
		if (m_allocator) {
			vmaDestroyAllocator(m_allocator);
			m_allocator = nullptr;
		}

		if (m_device) {
			vkDeviceWaitIdle(m_device);
			vkDestroyDevice(m_device, nullptr);
			m_device = nullptr;
		}

		if (m_debugMessenger) {
			vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
			m_debugMessenger = nullptr;
		}

		if (m_instance) {
			vkDestroyInstance(m_instance, nullptr);
			m_instance = nullptr;
		}
	}

	UniquePtr<Buffer> VulkanDevice::CreateRHIBuffer(const BufferDesc& desc) 
	{
		UniquePtr<VulkanBuffer> pNewBuffer = MakeUnique<VulkanBuffer>();

		pNewBuffer->m_ownerDevice = this;
		pNewBuffer->m_size = desc.size;
		pNewBuffer->m_usage = desc.usage;
		pNewBuffer->m_memory = desc.memory;
 
		VkBufferUsageFlags usage{};
		if (HasFlag(desc.usage, BufferUsage::Vertex))
			usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (HasFlag(desc.usage, BufferUsage::Index))
			usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (HasFlag(desc.usage, BufferUsage::Constant))
			usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (HasFlag(desc.usage, BufferUsage::Storage))
			usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		if (HasFlag(desc.usage, BufferUsage::Indirect))
			usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (HasFlag(desc.usage, BufferUsage::CopySrc))
			usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (HasFlag(desc.usage, BufferUsage::CopyDst))
			usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// TODO: change name
		List<u32> uniqueQueueFamilies;
		for (const u32 queueFamily : m_queueFamilies)
		{
			if (!uniqueQueueFamilies.Contains(queueFamily))
				uniqueQueueFamilies.EmplaceBack(queueFamily);
		}
		
		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = desc.size;
		createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = uniqueQueueFamilies.GetCount();
		createInfo.pQueueFamilyIndices = uniqueQueueFamilies.GetData();
		createInfo.usage = usage;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.requiredFlags = desc.memory != MemoryType::GpuOnly ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VkMemoryPropertyFlagBits{};
		allocCreateInfo.usage = desc.memory == MemoryType::GpuUpload ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE : VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = desc.memory != MemoryType::GpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlagBits{};

		if (desc.memory == MemoryType::GpuUpload || desc.memory == MemoryType::Upload)
			allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		if (desc.memory == MemoryType::Readback)
			allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

		VmaAllocationInfo allocInfo{};

		vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo, &pNewBuffer->m_buffer, &pNewBuffer->m_allocation, &allocInfo);
		pNewBuffer->m_mappedData = reinterpret_cast<u8 *>(allocInfo.pMappedData);

		pNewBuffer->m_gpuAddress = 0;

		if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			VkBufferDeviceAddressInfo addressInfo{};
			addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			addressInfo.buffer = pNewBuffer->m_buffer;

			pNewBuffer->m_gpuAddress = vkGetBufferDeviceAddress(m_device, &addressInfo);
		}

		return pNewBuffer;
	}
	UniquePtr<Texture> VulkanDevice::CreateRHITexture(const TextureDesc& desc) 
	{
		UniquePtr<VulkanTexture> pNewTexture = MakeUnique<VulkanTexture>();

		pNewTexture->m_ownerDevice = this;
		pNewTexture->m_type = desc.type;
		pNewTexture->m_format = desc.format;
		pNewTexture->m_usage = desc.usage;
		pNewTexture->m_width = desc.width;
		pNewTexture->m_height = desc.type == TextureType::Texture1D ? 1 : desc.height;
		pNewTexture->m_depth = desc.type == TextureType::Texture3D ? desc.depth : 1;
		pNewTexture->m_arrayLayers = desc.type == TextureType::Texture3D ? 1 : desc.arrayLayers;
		pNewTexture->m_mipLevels = desc.mipLevels;
		pNewTexture->m_sampleCount = desc.sampleCount;

		VkImageUsageFlags usage{};
		if (HasFlag(desc.usage, TextureUsage::CopyDst))
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (HasFlag(desc.usage, TextureUsage::CopySrc))
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (HasFlag(desc.usage, TextureUsage::DepthStencil))
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (HasFlag(desc.usage, TextureUsage::RenderTarget))
			usage |= (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		if (HasFlag(desc.usage, TextureUsage::DepthStencil))
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (HasFlag(desc.usage, TextureUsage::Sampled))
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (HasFlag(desc.usage, TextureUsage::Storage))
			usage |= VK_IMAGE_USAGE_STORAGE_BIT;

		// TODO: change name
		List<u32> uniqueQueueFamilies;
		for (const u32 queueFamily : m_queueFamilies)
		{
			if (!uniqueQueueFamilies.Contains(queueFamily))
				uniqueQueueFamilies.EmplaceBack(queueFamily);
		}

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = Helpers::ToFormat(desc.format);
		imageCreateInfo.extent.width = desc.width;
		imageCreateInfo.extent.height = desc.height;
		imageCreateInfo.extent.depth = desc.depth;
		imageCreateInfo.mipLevels = desc.mipLevels;
		imageCreateInfo.arrayLayers = desc.arrayLayers;
		// TODO: temp
		imageCreateInfo.samples = static_cast<VkSampleCountFlagBits>(desc.sampleCount);
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		imageCreateInfo.queueFamilyIndexCount = uniqueQueueFamilies.GetCount();
		imageCreateInfo.pQueueFamilyIndices = uniqueQueueFamilies.GetData();

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		vmaCreateImage(m_allocator, &imageCreateInfo, &allocCreateInfo, &pNewTexture->m_image, &pNewTexture->m_allocation, nullptr);
		
		return pNewTexture;
	}
	UniquePtr<TextureView> VulkanDevice::CreateRHITextureView(const TextureViewDesc& desc) 
	{ 
		if (!desc.pTexture)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "TextureView needs a texture");
			return nullptr;
		}

		VulkanTexture* pTexture = static_cast<VulkanTexture*>(desc.pTexture);

		u32 mipCount = desc.mipCount == kInvalid32 ? pTexture->GetMipLevels() - desc.baseMip : desc.mipCount;
		u32 layerCount = desc.layerCount == kInvalid32 ? pTexture->GetArrayLayers() - desc.baseLayer : desc.layerCount;

		b8 isMipOutOfRange = desc.baseMip >= pTexture->GetMipLevels() || desc.baseMip + mipCount > pTexture->GetMipLevels();
		b8 isLayerOutOfRange = desc.baseLayer >= pTexture->GetArrayLayers() || desc.baseLayer + layerCount > pTexture->GetArrayLayers();
		if (isMipOutOfRange || isLayerOutOfRange)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "View range is out of texture, mips: {}+{}/{}, layers: {}+{}/{}",
				desc.baseMip, mipCount, pTexture->GetMipLevels(), desc.baseLayer, layerCount, pTexture->GetArrayLayers());
			return nullptr;
		}

		UniquePtr<VulkanTextureView> pNewView = MakeUnique<VulkanTextureView>();

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = pTexture->Handle();
		viewCreateInfo.format = Helpers::ToFormat(pTexture->GetFormat());
		viewCreateInfo.subresourceRange.aspectMask 
			= desc.aspect == TextureAspect::All ? Helpers::ToAspectMask(pTexture->GetFormat()) 
				: desc.aspect == TextureAspect::Depth ? VK_IMAGE_ASPECT_DEPTH_BIT
				: desc.aspect == TextureAspect::Stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
		viewCreateInfo.subresourceRange.baseMipLevel = desc.baseMip;
		viewCreateInfo.subresourceRange.levelCount = mipCount;
		viewCreateInfo.subresourceRange.baseArrayLayer = desc.baseLayer;
		viewCreateInfo.subresourceRange.layerCount = layerCount;

		switch (desc.type)
		{
			case TextureViewType::Texture1D:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
				break;
			case TextureViewType::Texture1DArray:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				break;
			case TextureViewType::Texture2D:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				break;
			case TextureViewType::Texture2DArray:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				break;
			case TextureViewType::Texture3D:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
				break;
			case TextureViewType::TextureCube:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				break;
			case TextureViewType::TextureCubeArray:
				viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
				break;
			default: viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		}

		pNewView->m_ownerDevice = this;
		pNewView->m_texture = desc.pTexture;
		pNewView->m_type = desc.type;
		pNewView->m_format = desc.format == TextureFormat::Undefined ? pTexture->GetFormat() : desc.format;
		pNewView->m_aspect = desc.aspect;
		pNewView->m_baseMip = desc.baseMip;
		pNewView->m_mipCount = mipCount;
		pNewView->m_baseLayer = desc.baseLayer;
		pNewView->m_layerCount = layerCount;
		pNewView->m_createInfo = viewCreateInfo; 

		VkResult result = vkCreateImageView(m_device, &viewCreateInfo, nullptr, &pNewView->m_view);
		if (result != VK_SUCCESS) {
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkImageView, VkResult({})", (i32)result);
			return nullptr;
		}

		return pNewView;
	}
	UniquePtr<Sampler> VulkanDevice::CreateRHISampler(const SamplerDesc& desc) 
	{
		UniquePtr<VulkanSampler> pNewSampler = MakeUnique<VulkanSampler>();

		pNewSampler->m_ownerDevice = this;
		pNewSampler->m_desc = desc;
		pNewSampler->m_samplerCreateInfo = Helpers::ToSamplerCreateInfo(desc);

		return pNewSampler; 
	}
	UniquePtr<ShaderModule> VulkanDevice::CreateRHIShaderModule(const ShaderModuleDesc& desc) 
	{ 
		if (desc.byteCode.GetCount() < 4)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShaderModule has no bytecode, entry: {}", desc.entryPoint);
			return nullptr;
		}

		UniquePtr<VulkanShaderModule> pNewModule = MakeUnique<VulkanShaderModule>();

		pNewModule->m_ownerDevice = this;
		pNewModule->m_stage = desc.stage;
		pNewModule->m_entryPoint = desc.entryPoint;
		pNewModule->m_byteCode = List<u8>(desc.byteCode.begin(), desc.byteCode.end());

		return pNewModule;
	}
	UniquePtr<PipelineLayout> VulkanDevice::CreateRHIPipelineLayout(const PipelineLayoutDesc& desc) 
	{
		if (desc.bindless && !desc.tables.IsEmpty())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Bindless layout cannot have tables, got: {}", desc.tables.GetCount());
			return nullptr;
		}

		if (desc.pushConstantSize % 4 != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Push constant size must be a multiple of 4, got: {}", desc.pushConstantSize);
			return nullptr;
		}

		if (desc.pushConstantSize / 4 + desc.tables.GetCount() > 64)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Root signature exceeds 64 DWORDs, push: {} tables: {}", desc.pushConstantSize / 4, desc.tables.GetCount());
			return nullptr;
		}
 
		UniquePtr<VulkanPipelineLayout> pNewLayout = MakeUnique<VulkanPipelineLayout>();

		pNewLayout->m_ownerDevice = this;
		pNewLayout->m_tables = desc.tables;
		pNewLayout->m_pushConstantSize = desc.pushConstantSize;
		pNewLayout->m_bindless = desc.bindless;

		if (pNewLayout->m_bindless)
		{
			VkDescriptorSetAndBindingMappingEXT resourceMapping{};
			resourceMapping.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
			resourceMapping.descriptorSet = 0;
			resourceMapping.bindingCount = 1;
			resourceMapping.resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT;
			resourceMapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
			resourceMapping.sourceData.constantOffset.heapOffset = 0;
			resourceMapping.sourceData.constantOffset.heapArrayStride = m_resourceDescriptorSize;

			VkDescriptorSetAndBindingMappingEXT samplerMapping{};
			samplerMapping.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
			samplerMapping.descriptorSet = 1;
			samplerMapping.bindingCount = 1;
			samplerMapping.resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT;
			samplerMapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
			samplerMapping.sourceData.constantOffset.samplerHeapOffset = 0;
			samplerMapping.sourceData.constantOffset.samplerHeapArrayStride = m_samplerDescriptorSize;

			pNewLayout->m_mappings.PushBack(resourceMapping);
			pNewLayout->m_mappings.PushBack(samplerMapping);

			return pNewLayout;
		}

		u32 set{};
		u32 resourceHeapOffset{};
		u32 samplerHeapOffset{};
		for (auto &table : pNewLayout->m_tables) 
		{
			VkDescriptorSetAndBindingMappingEXT mapping{};
			mapping.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
			mapping.descriptorSet = set++;
			mapping.bindingCount = 1;
			mapping.resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT;
			mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
			mapping.sourceData.pushIndex.pushOffset = 256 - (sizeof(u32) * (set + 1));
			
			std::ranges::sort(table.ranges, [](const DescriptorRange& a, const DescriptorRange& b) { return a.binding < b.binding; });

			for (const auto &descriptorRange : table.ranges) 
			{
				mapping.firstBinding = descriptorRange.binding;

				if (descriptorRange.type == DescriptorType::Sampler)
				{
					mapping.sourceData.pushIndex.samplerHeapOffset = samplerHeapOffset;
					samplerHeapOffset += descriptorRange.count * m_samplerDescriptorSize;
					mapping.sourceData.pushIndex.heapIndexStride = m_samplerDescriptorSize;
				}
				else 
				{
					mapping.sourceData.pushIndex.heapOffset = resourceHeapOffset;
					resourceHeapOffset += descriptorRange.count * m_resourceDescriptorSize;
					mapping.sourceData.pushIndex.heapIndexStride = m_resourceDescriptorSize;
				}
			}

			pNewLayout->m_mappings.PushBack(mapping);
		}

		return pNewLayout;
	}
	UniquePtr<GraphicsPipeline> VulkanDevice::CreateRHIPipeline(const GraphicsPipelineDesc& desc) 
	{
		if (!desc.pLayout)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline needs a layout");
			return nullptr;
		}

		b8 isMeshPipeline = desc.pMeshShader != nullptr;
		if (isMeshPipeline == (desc.pVertexShader != nullptr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline needs either a vertex or a mesh shader, not both or none");
			return nullptr;
		}

		if (desc.colorTargets.GetCount() > kMaxColorAttachments)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsPipeline has {} color targets, max: {}", desc.colorTargets.GetCount(), kMaxColorAttachments);
			return nullptr;
		}

		List<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
		colorBlendAttachments.Reserve(desc.colorTargets.GetCount());
		for (const ColorTarget& target : desc.colorTargets) {
			colorBlendAttachments.EmplaceBack(
				target.blend.enable,
				Helpers::ToBlendFactor(target.blend.srcColor),
				Helpers::ToBlendFactor(target.blend.dstColor),
				Helpers::ToBlendOp(target.blend.colorOp),
				Helpers::ToBlendFactor(target.blend.srcAlpha),
				Helpers::ToBlendFactor(target.blend.dstAlpha),
				Helpers::ToBlendOp(target.blend.alphaOp)
			);
		}

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = desc.colorTargets.GetCount();
		colorBlendState.pAttachments = colorBlendAttachments.GetData();
		
		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = desc.depthStencil.depthTest;
		depthStencilState.depthWriteEnable = desc.depthStencil.depthWrite;
		depthStencilState.depthCompareOp = Helpers::ToCompareOp(desc.depthStencil.depthCompare);

		depthStencilState.stencilTestEnable = desc.depthStencil.stencilTest;

		depthStencilState.front.compareMask = desc.depthStencil.stencilReadMask;
		depthStencilState.front.writeMask = desc.depthStencil.stencilWriteMask;
		depthStencilState.front.compareOp = Helpers::ToCompareOp(desc.depthStencil.front.compareOp);
		depthStencilState.front.failOp = Helpers::ToStencilOp(desc.depthStencil.front.failOp);
		depthStencilState.front.passOp = Helpers::ToStencilOp(desc.depthStencil.front.passOp);
		depthStencilState.front.depthFailOp = Helpers::ToStencilOp(desc.depthStencil.front.depthFailOp);

		depthStencilState.back.compareMask = desc.depthStencil.stencilReadMask;
		depthStencilState.back.writeMask = desc.depthStencil.stencilWriteMask;
		depthStencilState.back.compareOp = Helpers::ToCompareOp(desc.depthStencil.back.compareOp);
		depthStencilState.back.failOp = Helpers::ToStencilOp(desc.depthStencil.back.failOp);
		depthStencilState.back.passOp = Helpers::ToStencilOp(desc.depthStencil.back.passOp);
		depthStencilState.back.depthFailOp = Helpers::ToStencilOp(desc.depthStencil.back.depthFailOp);

		List<VkDynamicState> dynamicStates{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicStateState{};
		dynamicStateState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateState.dynamicStateCount = dynamicStates.GetCount();
		dynamicStateState.pDynamicStates = dynamicStates.GetData();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = Helpers::ToPrimitiveTopology(desc.topology);

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = static_cast<VkSampleCountFlagBits>(desc.sampleCount);
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 1.0f;
		multisampleState.alphaToCoverageEnable = desc.alphaToCoverage;

		VkPipelineRasterizationStateCreateInfo  rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = Helpers::ToPolygonMode(desc.rasterizer.fillMode);
		rasterizationState.cullMode = Helpers::ToCullMode(desc.rasterizer.cullMode);
		rasterizationState.frontFace = Helpers::ToFrontFace(desc.rasterizer.frontFace);
		rasterizationState.depthClampEnable = desc.rasterizer.depthClip;
		rasterizationState.depthBiasEnable = desc.rasterizer.depthBias;
		rasterizationState.depthBiasConstantFactor = desc.rasterizer.depthBiasClamp;
		rasterizationState.depthBiasSlopeFactor = desc.rasterizer.slopeScaledDepthBias;
		rasterizationState.lineWidth = 1.0f;

		List<VkVertexInputBindingDescription> vertexBindingDesc;
		vertexBindingDesc.Reserve(desc.vertexLayout.bindings.GetCount());
		for (const VertexBinding& binding : desc.vertexLayout.bindings)
		{
			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = binding.binding;
			bindingDesc.stride = binding.stride;
			bindingDesc.inputRate = Helpers::ToVertexInputRate(binding.inputRate);
			vertexBindingDesc.EmplaceBack(bindingDesc);
		}

		List<VkVertexInputAttributeDescription> vertexAttributeDesc;
		vertexAttributeDesc.Reserve(desc.vertexLayout.attributes.GetCount());
		for (const VertexAttribute& attribute : desc.vertexLayout.attributes)
		{
			VkVertexInputAttributeDescription attributeDesc{};
			attributeDesc.binding = attribute.binding;
			attributeDesc.location = attribute.semanticIndex;
			attributeDesc.format = Helpers::ToFormat(attribute.format);
			attributeDesc.offset = attribute.offset;
			vertexAttributeDesc.EmplaceBack(attributeDesc);
		}

		VkPipelineVertexInputStateCreateInfo vertexBindingState{};
		vertexBindingState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexBindingState.vertexBindingDescriptionCount = vertexBindingDesc.GetCount();
		vertexBindingState.pVertexBindingDescriptions = vertexBindingDesc.GetData();
		vertexBindingState.vertexAttributeDescriptionCount = vertexAttributeDesc.GetCount();
		vertexBindingState.pVertexAttributeDescriptions = vertexAttributeDesc.GetData();

		VkPipelineCreateFlags2CreateInfo pipelineFlags{};
		pipelineFlags.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO;
		pipelineFlags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

		List<VkFormat> colorAttachmentFormats;
		colorAttachmentFormats.Reserve(desc.colorTargets.GetCount());
		for (const ColorTarget& target : desc.colorTargets) 
		{
			colorAttachmentFormats.EmplaceBack(Helpers::ToFormat(target.format));
		}

		VkPipelineRenderingCreateInfo renderingState{};
		renderingState.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingState.colorAttachmentCount = colorAttachmentFormats.GetCount();
		renderingState.pColorAttachmentFormats = colorAttachmentFormats.GetData();
		renderingState.depthAttachmentFormat = Helpers::ToFormat(desc.depthFormat);
		renderingState.stencilAttachmentFormat = Helpers::IsStencilFormat(desc.depthFormat) ? Helpers::ToFormat(desc.depthFormat) : VK_FORMAT_UNDEFINED;
		renderingState.pNext = &pipelineFlags;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkShaderDescriptorSetAndBindingMappingInfoEXT mappingInfo = static_cast<VulkanPipelineLayout*>(desc.pLayout)->GetMappingInfo();

		VkShaderModuleCreateInfo vertexOrMeshShaderModule{};
		vertexOrMeshShaderModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexOrMeshShaderModule.codeSize = isMeshPipeline ? desc.pMeshShader->GetByteCode().GetCount() : desc.pVertexShader->GetByteCode().GetCount();
		vertexOrMeshShaderModule.pCode = reinterpret_cast<const uint32_t*>(isMeshPipeline ? desc.pMeshShader->GetByteCode().GetData() : desc.pVertexShader->GetByteCode().GetData());
		vertexOrMeshShaderModule.pNext = &mappingInfo;

		VkShaderModuleCreateInfo fragmentShaderModule{};
		if (desc.pPixelShader)
		{
			fragmentShaderModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			fragmentShaderModule.codeSize = desc.pPixelShader->GetByteCode().GetCount();
			fragmentShaderModule.pCode = reinterpret_cast<const uint32_t*>(desc.pPixelShader->GetByteCode().GetData());
			fragmentShaderModule.pNext = &mappingInfo;
		}

		VkShaderModuleCreateInfo taskShaderModule{};
		if (desc.pTaskShader)
		{
			taskShaderModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			taskShaderModule.codeSize = desc.pTaskShader->GetByteCode().GetCount();
			taskShaderModule.pCode = reinterpret_cast<const uint32_t*>(desc.pTaskShader->GetByteCode().GetData());
			taskShaderModule.pNext = &mappingInfo;
		}

		VkPipelineShaderStageCreateInfo shaderStages[3]{};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = isMeshPipeline ? VK_SHADER_STAGE_MESH_BIT_EXT : VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = isMeshPipeline ? desc.pMeshShader->GetEntryPoint().c_str() : desc.pVertexShader->GetEntryPoint().c_str();
		shaderStages[0].pNext = &vertexOrMeshShaderModule;

		if (desc.pPixelShader) 
		{
			shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStages[1].pName = desc.pPixelShader->GetEntryPoint().c_str();
			shaderStages[1].pNext =  &fragmentShaderModule;
		}

		if (desc.pTaskShader)
		{
			shaderStages[b8(desc.pPixelShader) + 1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStages[b8(desc.pPixelShader) + 1].stage = VK_SHADER_STAGE_TASK_BIT_EXT;
			shaderStages[b8(desc.pPixelShader) + 1].pName = desc.pTaskShader->GetEntryPoint().c_str();
			shaderStages[b8(desc.pPixelShader) + 1].pNext = &taskShaderModule;
		}
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicStateState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pVertexInputState = &vertexBindingState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.stageCount = 1 + b8(desc.pTaskShader) + b8(desc.pPixelShader);
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pNext = &renderingState;

		UniquePtr<VulkanGraphicsPipeline> pNewPipeline = MakeUnique<VulkanGraphicsPipeline>();

		pNewPipeline->m_ownerDevice = this;
		pNewPipeline->m_type = PipelineType::Graphics;
		pNewPipeline->m_layout = desc.pLayout;
		pNewPipeline->m_topology = desc.topology;
		pNewPipeline->m_vertexBindings = desc.vertexLayout.bindings;
		pNewPipeline->m_isMeshPipeline = isMeshPipeline;

		vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pNewPipeline->m_pipeline);

		return pNewPipeline;
	}

	UniquePtr<ComputePipeline> VulkanDevice::CreateRHIPipeline(const ComputePipelineDesc& desc) 
	{ 
		if (!desc.pLayout || !desc.pComputeShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ComputePipeline needs a layout and a compute shader");
			return nullptr;
		}

		UniquePtr<VulkanComputePipeline> pNewPipeline = MakeUnique<VulkanComputePipeline>();

		pNewPipeline->m_ownerDevice = this;
		pNewPipeline->m_type = PipelineType::Compute;
		pNewPipeline->m_layout = desc.pLayout;

		auto *pLayout = static_cast<VulkanPipelineLayout*>(desc.pLayout);
		VkShaderDescriptorSetAndBindingMappingInfoEXT mappingInfo = pLayout->GetMappingInfo();

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = desc.pComputeShader->GetByteCode().GetCount();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(desc.pComputeShader->GetByteCode().GetData());	
		shaderModuleCreateInfo.pNext = &mappingInfo;

		VkPipelineShaderStageCreateInfo stageCreateInfo{};
		stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageCreateInfo.pName = desc.pComputeShader->GetEntryPoint().c_str();
		stageCreateInfo.pNext = &shaderModuleCreateInfo;

		VkPipelineCreateFlags2CreateInfo pipelineFlags{};
		pipelineFlags.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO;
		pipelineFlags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

		VkComputePipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.stage = stageCreateInfo;
		createInfo.pNext = &pipelineFlags;

		VkResult result = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pNewPipeline->m_pipeline);

		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkComputePipeline, VkResult({})", (i32)result);
			return nullptr;
		}

		return pNewPipeline;
	}
	UniquePtr<DescriptorHeap> VulkanDevice::CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc) 
	{ 
		if (desc.capacity == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Descriptor heap capacity is zero");
			return nullptr;
		}

		// TODO:
		// if (desc.type == DescriptorHeapType::Sampler && desc.capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE)
		// {
		// 	Terminal::Error(StringHelper::GetTypeName(this), "Sampler heap capacity {} exceeds {}", desc.capacity, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);
		// 	return nullptr;
		// }

		UniquePtr<VulkanDescriptorHeap> pNewHeap = MakeUnique<VulkanDescriptorHeap>();

		pNewHeap->m_ownerDevice = this;
		pNewHeap->m_type = desc.type;
		pNewHeap->m_capacity = desc.capacity;

		if (desc.type == DescriptorHeapType::Sampler)
		{
			pNewHeap->m_stride = m_samplerDescriptorSize;
			pNewHeap->m_reservedForDriver = m_samplerHeapReservedRange;
		}
		else
		{
			pNewHeap->m_stride = m_resourceDescriptorSize;
			pNewHeap->m_reservedForDriver = m_resourceHeapReservedRange;
		}

		VkBufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = (desc.capacity * pNewHeap->m_stride) + pNewHeap->m_reservedForDriver;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.usage = VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo{};

		vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo, &pNewHeap->m_heap, &pNewHeap->m_allocation, &allocInfo);
		pNewHeap->m_mappedPtr = reinterpret_cast<u8 *>(allocInfo.pMappedData);

		const VkBufferDeviceAddressInfo addressInfo{
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			nullptr,
			pNewHeap->m_heap
		};

		pNewHeap->m_gpuAddress = vkGetBufferDeviceAddress(m_device, &addressInfo);

		pNewHeap->m_freeBlocks.PushBack({ 0, desc.capacity });

		return pNewHeap;
	}
	UniquePtr<DescriptorTable> VulkanDevice::CreateRHIDescriptorTable(const DescriptorTableDesc& desc) 
	{ 
		if (!desc.pHeap || !desc.pLayout)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DescriptorTable needs a heap and a layout");
			return nullptr;
		}

		const List<DescriptorTableLayout>& tables = desc.pLayout->GetTables();
		if (desc.tableIndex >= tables.GetCount())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Layout has no table {}, table count: {}", desc.tableIndex, tables.GetCount());
			return nullptr;
		}

		const DescriptorTableLayout& layout = tables[desc.tableIndex];
		b8 isSamplerTable = layout.ranges[0].type == DescriptorType::Sampler;
		b8 isSamplerHeap = desc.pHeap->GetType() == DescriptorHeapType::Sampler;
		if (isSamplerTable != isSamplerHeap)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Table {} does not match the heap type", desc.tableIndex);
			return nullptr;
		}

		UniquePtr<VulkanDescriptorTable> pNewTable = MakeUnique<VulkanDescriptorTable>();
		VulkanDescriptorHeap* pHeap = static_cast<VulkanDescriptorHeap*>(desc.pHeap);
		const VulkanPipelineLayout* pLayout = static_cast<VulkanPipelineLayout*>(desc.pLayout);

		pNewTable->m_ownerDevice = this;
		pNewTable->m_heap = desc.pHeap;
		pNewTable->m_layout = desc.pLayout;
		pNewTable->m_tableIndex = desc.tableIndex;
		pNewTable->m_variableCount = desc.variableCount;

		u32 offset{};

		u32 i{};
		for (const auto& desc : pLayout->GetTables()[desc.tableIndex].ranges)
		{
			pNewTable->m_slots.EmplaceBack(desc.type, desc.binding, offset);
			offset += desc.count;
		}

		pNewTable->m_offset = pHeap->AllocateRange(offset);

		return pNewTable;
	}
	UniquePtr<CommandPool> VulkanDevice::CreateRHICommandPool(const CommandPoolDesc& desc) 
	{ 
		UniquePtr<VulkanCommandPool> pNewPool = MakeUnique<VulkanCommandPool>();

		pNewPool->m_ownerDevice = this;
		pNewPool->m_type = desc.type;

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = m_queueFamilies[static_cast<u32>(desc.type)];
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkResult result = vkCreateCommandPool(m_device, &createInfo, nullptr, &pNewPool->m_cmdPool);
		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkCommandPool, VkResult({})", (i32)result);
			return nullptr;
		}

		return pNewPool;
	}
	UniquePtr<CommandList> VulkanDevice::CreateRHICommandList(const CommandListDesc& desc) 
	{ 
		if (!desc.pPool)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "CommandList needs a pool");
			return nullptr;
		}

		UniquePtr<VulkanCommandList> pNewList = MakeUnique<VulkanCommandList>();

		pNewList->m_ownerDevice = this;
		pNewList->m_pool = desc.pPool;
		pNewList->m_type = desc.pPool->GetType();

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = static_cast<VulkanCommandPool*>(desc.pPool)->Handle();
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(m_device, &allocateInfo, &pNewList->m_cmdBuffer);
		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to allocate VkCommandBuffer, VkResult({})", (i32)result);
			return nullptr;
		}

		return pNewList; 
	}
	UniquePtr<Fence> VulkanDevice::CreateRHIFence(const FenceDesc& desc) 
	{ 
		UniquePtr<VulkanFence> pNewFence = MakeUnique<VulkanFence>();

		pNewFence->m_ownerDevice = this;
		pNewFence->m_value = desc.initialValue;

		const VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = desc.initialValue
		};

		const VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeCreateInfo,
		};

		const VkResult result = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &pNewFence->m_semaphore);
		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkSemaphore, VkResult({})", (i32)result);
			return nullptr;
		}

		return pNewFence; 
	}
	UniquePtr<Swapchain> VulkanDevice::CreateRHISwapchain(const SwapchainDesc& desc) 
	{ 
		if (desc.windowHandle == PAL::OSHandle{} || desc.width == 0 || desc.height == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Swapchain needs a window and a size, size: {}x{}", desc.width, desc.height);
			return nullptr;
		}

		if (desc.imageCount < 2 || desc.imageCount > kMaxSwapchainImages)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Swapchain image count must be in [2, {}], got: {}", kMaxSwapchainImages, desc.imageCount);
			return nullptr;
		}

		UniquePtr<VulkanSwapchain> pNewSwapchain = MakeUnique<VulkanSwapchain>();

		pNewSwapchain->m_ownerDevice = this;
		pNewSwapchain->m_format = desc.format;
		pNewSwapchain->m_presentMode = desc.presentMode;
		pNewSwapchain->m_width = desc.width;
		pNewSwapchain->m_height = desc.height;
		pNewSwapchain->m_imageCount = desc.imageCount;
		// TODO: when possible use VK_PRESENT_MODE_MAILBOX_KHR or VK_PRESENT_MODE_FIFO_RELAXED_KHR for low latency and no tearing 
		pNewSwapchain->m_vkPresentMode = desc.presentMode == PresentMode::Vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

#if defined(HORIZON_WINDOWS)
		VkWin32SurfaceCreateInfoKHR win32SurfaceCreate{};
		win32SurfaceCreate.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32SurfaceCreate.hinstance = GetModuleHandle(nullptr);
		win32SurfaceCreate.hwnd = reinterpret_cast<HWND>(desc.windowHandle);

		vkCreateWin32SurfaceKHR(m_instance, &win32SurfaceCreate, nullptr, &pNewSwapchain->m_surface);
#elif defined(HORIZON_LINUX)
		#error i dont complate linux surface creation
#endif

		pNewSwapchain->m_presentSemaphores.Resize(pNewSwapchain->m_imageCount);
		VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
		vkCreateFence(m_device, &createInfo, nullptr, &pNewSwapchain->m_acquireFence);

		for (u32 i{}; i < pNewSwapchain->m_imageCount; i++) {
			VkSemaphoreCreateInfo createInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			vkCreateSemaphore(m_device, &createInfo, nullptr, &pNewSwapchain->m_presentSemaphores[i]);
		}

		pNewSwapchain->ReCreateSwapchain();
		pNewSwapchain->CreateImages();

		return pNewSwapchain;
	}

	void VulkanDevice::Submit(QueueType type, ReadOnlyList<CommandList*> lists) {
		List<VkCommandBufferSubmitInfo> cmdSubmits;
		List<VkSemaphoreSubmitInfo> waitSemaphores;
		List<VkSemaphoreSubmitInfo> signalSemaphores;
		cmdSubmits.Reserve(lists.GetCount());
		waitSemaphores.Reserve(lists.GetCount());
		signalSemaphores.Reserve(lists.GetCount());

		for (const CommandList* cmdList : lists) {
			const VulkanCommandList* vkCmdList = static_cast<const VulkanCommandList*>(cmdList);
			cmdSubmits.EmplaceBack(
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				nullptr,
				vkCmdList->Handle()
			);
		}

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.pCommandBufferInfos = cmdSubmits.GetData();
		submitInfo.commandBufferInfoCount = cmdSubmits.GetCount();
		submitInfo.pWaitSemaphoreInfos = waitSemaphores.GetData();
		submitInfo.waitSemaphoreInfoCount = waitSemaphores.GetCount();
		submitInfo.pSignalSemaphoreInfos = signalSemaphores.GetData();
		submitInfo.signalSemaphoreInfoCount = signalSemaphores.GetCount();

		vkQueueSubmit2(m_queues[static_cast<u32>(type)], 1, &submitInfo, nullptr);
	}

	u64 VulkanDevice::Signal(QueueType type, Fence* pFence) {
		VulkanFence* vkFence = static_cast<VulkanFence*>(pFence);
		u64 value = vkFence->Advance();

		VkSemaphoreSubmitInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = vkFence->Handle();
		signalInfo.value = value;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.signalSemaphoreInfoCount = 1;

		vkQueueSubmit2(m_queues[static_cast<u32>(type)], 1, &submitInfo, nullptr);
		return value;
	}
	void VulkanDevice::Wait(QueueType type, Fence* pFence, u64 value) {
		VkSemaphoreSubmitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = static_cast<VulkanFence*>(pFence)->Handle();
		waitInfo.value = value;

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.waitSemaphoreInfoCount = 1;

		vkQueueSubmit2(m_queues[static_cast<u32>(type)], 1, &submitInfo, nullptr);
	}
	void VulkanDevice::WaitIdle() {
		vkDeviceWaitIdle(m_device);
	}

	b8 VulkanDevice::CreateInstance()
	{
		const auto extensions = GetInstanceExtensions();
		const auto layers = GetInstanceLayers();

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_API_VERSION_1_4;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.pEngineName = "Horizon Engine";
		appInfo.pApplicationName = "";

		List<VkValidationFeatureEnableEXT> enabledFeatures;

		if (m_deviceDesc.enableGpuValidation)
			enabledFeatures.EmplaceBack(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);

		if (m_deviceDesc.synchronizedCommandValidation)
			enabledFeatures.EmplaceBack(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);

		List<VkValidationFeatureDisableEXT> disabledFeatures;
		// nsight not work with VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
		disabledFeatures.EmplaceBack(VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT);

		VkValidationFeaturesEXT validationFeatures{};
		validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		validationFeatures.pEnabledValidationFeatures = enabledFeatures.GetData();
		validationFeatures.enabledValidationFeatureCount = enabledFeatures.GetCount();
		validationFeatures.pDisabledValidationFeatures = disabledFeatures.GetData();
		validationFeatures.disabledValidationFeatureCount = disabledFeatures.GetCount();

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledExtensionCount = extensions.GetCount();
		instanceCreateInfo.ppEnabledExtensionNames = extensions.GetData();
		instanceCreateInfo.enabledLayerCount = layers.GetCount();
		instanceCreateInfo.ppEnabledLayerNames = layers.GetData();
		instanceCreateInfo.pNext = m_deviceDesc.enableDebug ? &validationFeatures : nullptr;

		const auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
		if (result != VK_SUCCESS) {
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkInstance, VkResult({})", (i32)result);
			return false;
		}

		return true;
	}

	b8 VulkanDevice::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerCreateInfo.pfnUserCallback = DebugCallback;
		debugMessengerCreateInfo.pUserData = nullptr;

		debugMessengerCreateInfo.messageSeverity
			= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		debugMessengerCreateInfo.messageType
			= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

		const auto result = vkCreateDebugUtilsMessengerEXT(m_instance, &debugMessengerCreateInfo, nullptr, &m_debugMessenger);
		if (result != VK_SUCCESS) {
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkDebugUtilsMessengerEXT, VkResult({})", (i32)result);
			return false;
		}

		return true;
	}

	b8 VulkanDevice::PickPhysicalDevice()
	{
		u32 deviceCount{};
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		List<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.GetData());

		for (const auto device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties{};
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			Terminal::Info(StringHelper::GetTypeName(this), "Eveluting device: {}", deviceProperties.deviceName);

			if (!CheckDeviceSupport(device))
				continue;

			m_physicalDevice = device;

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				break;
		}

		return m_physicalDevice != VK_NULL_HANDLE;
	}

	List<VkDeviceQueueCreateInfo> VulkanDevice::PickQueueFamilies()
	{
        const float queuePriorities = 1.f;

        VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
        VkDeviceQueueCreateInfo computeQueueCreateInfo{};
        VkDeviceQueueCreateInfo transferQueueCreateInfo{};

		graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		transferQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

		graphicsQueueCreateInfo.pQueuePriorities = &queuePriorities;
		computeQueueCreateInfo.pQueuePriorities = &queuePriorities;
		transferQueueCreateInfo.pQueuePriorities = &queuePriorities;

        {
            m_queueFamilies[static_cast<u32>(QueueType::Graphics)] = -1;
            m_queueFamilies[static_cast<u32>(QueueType::Compute)] = -1;
            m_queueFamilies[static_cast<u32>(QueueType::Transfer)] = -1;

            u32 graphicsQueueCount{};
            u32 computeQueueCount{};

			u32 queueFamilyCount{};
			vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
			List<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.GetData());

            u32 familyIndex{};
            for (const auto queueFamily : queueFamilies) {
                const u32 count = queueFamily.queueCount;
                const VkQueueFlags flags = queueFamily.queueFlags;

                if (flags & VK_QUEUE_GRAPHICS_BIT) {
					m_queueFamilies[static_cast<u32>(QueueType::Graphics)] = familyIndex;
                    graphicsQueueCount = count;
                }
                else if (flags & VK_QUEUE_COMPUTE_BIT) {
                    m_queueFamilies[static_cast<u32>(QueueType::Compute)] = familyIndex;
                    computeQueueCount = count;
                }
                else if (flags & VK_QUEUE_TRANSFER_BIT
					&& !(flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
					&& !(flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
					&& !(flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
					) {
                    m_queueFamilies[static_cast<u32>(QueueType::Transfer)] = familyIndex;
                }
				++familyIndex;
            }

            graphicsQueueCreateInfo.queueCount = 1;
			graphicsQueueCreateInfo.queueFamilyIndex = m_queueFamilies[static_cast<u32>(QueueType::Graphics)];

			computeQueueCreateInfo.queueCount = 1;
			computeQueueCreateInfo.queueFamilyIndex = m_queueFamilies[static_cast<u32>(QueueType::Compute)];

			transferQueueCreateInfo.queueCount = 1;
			transferQueueCreateInfo.queueFamilyIndex = m_queueFamilies[static_cast<u32>(QueueType::Transfer)];

			if (m_queueFamilies[static_cast<u32>(QueueType::Compute)] == -1) {
				m_queueFamilies[static_cast<u32>(QueueType::Compute)] = m_queueFamilies[static_cast<u32>(QueueType::Graphics)];

				if (graphicsQueueCount > 1) ++graphicsQueueCreateInfo.queueCount;

				computeQueueCreateInfo.queueCount = 0;
			}

			if (m_queueFamilies[static_cast<u32>(QueueType::Transfer)] == -1) {
				const b8 useComputeQueue = computeQueueCreateInfo.queueCount != 0;
					m_queueFamilies[static_cast<u32>(QueueType::Transfer)] =
						useComputeQueue ? m_queueFamilies[static_cast<u32>(QueueType::Compute)] : m_queueFamilies[static_cast<u32>(QueueType::Graphics)];

				if (useComputeQueue && computeQueueCount > 1) ++computeQueueCreateInfo.queueCount;
				else if (!useComputeQueue && graphicsQueueCount > 1) ++graphicsQueueCreateInfo.queueCount;

				transferQueueCreateInfo.queueCount = 0;
			}
        }

        List<VkDeviceQueueCreateInfo> queues;

        queues.EmplaceBack(graphicsQueueCreateInfo);
        if (computeQueueCreateInfo.queueCount) queues.EmplaceBack(computeQueueCreateInfo);
        if (transferQueueCreateInfo.queueCount) queues.EmplaceBack(transferQueueCreateInfo);

		return queues;
	}

	b8 VulkanDevice::CreateDevice()
	{
		auto queueCreateInfos = PickQueueFamilies();

		List<const c8*> deviceExtensions = GetDeviceExtensions();

		#define ADD_P_NEXT(features) features.pNext = pNext; \
			pNext = &features;

		void* pNext{};

		VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR swapchainMaintenace1Features{};
		swapchainMaintenace1Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR;
		swapchainMaintenace1Features.swapchainMaintenance1 = VK_TRUE;
		ADD_P_NEXT(swapchainMaintenace1Features);

		VkPhysicalDeviceShaderUntypedPointersFeaturesKHR shaderUntypedPointersFeatures{};
		shaderUntypedPointersFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR;
		shaderUntypedPointersFeatures.shaderUntypedPointers = VK_TRUE;
		ADD_P_NEXT(shaderUntypedPointersFeatures);

		VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeatures{};
		descriptorHeapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
		descriptorHeapFeatures.descriptorHeap = VK_TRUE;
		ADD_P_NEXT(descriptorHeapFeatures);

		VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR computeShaderDerivativesFeatures{};
		computeShaderDerivativesFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR;
		computeShaderDerivativesFeatures.computeDerivativeGroupQuads = VK_TRUE;
		computeShaderDerivativesFeatures.computeDerivativeGroupLinear = VK_TRUE;
		ADD_P_NEXT(computeShaderDerivativesFeatures);

		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
		meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
		meshShaderFeatures.meshShader = VK_TRUE;
		meshShaderFeatures.taskShader = VK_TRUE;
		ADD_P_NEXT(meshShaderFeatures);

		VkPhysicalDeviceVulkan14Features vulkan14Features{};
		vulkan14Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
		vulkan14Features.maintenance5 = VK_TRUE;
		ADD_P_NEXT(vulkan14Features);

		VkPhysicalDeviceVulkan13Features vulkan13Features{};
		vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13Features.dynamicRendering = VK_TRUE;
		vulkan13Features.maintenance4 = VK_TRUE;
		vulkan13Features.synchronization2 = VK_TRUE;
		vulkan13Features.shaderDemoteToHelperInvocation = VK_TRUE;
		vulkan13Features.shaderTerminateInvocation = VK_TRUE;
		vulkan13Features.subgroupSizeControl = VK_TRUE;
		vulkan13Features.computeFullSubgroups = VK_TRUE;
		ADD_P_NEXT(vulkan13Features);

		VkPhysicalDeviceVulkan12Features vulkan12Features{};
		vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12Features.descriptorIndexing = VK_TRUE;
		vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
		vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
		vulkan12Features.runtimeDescriptorArray = VK_TRUE;
		vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
		vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
		vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
		vulkan12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
		vulkan12Features.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
		vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		vulkan12Features.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
		vulkan12Features.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
		vulkan12Features.vulkanMemoryModel = VK_TRUE;
		vulkan12Features.vulkanMemoryModelDeviceScope = VK_TRUE;
		vulkan12Features.bufferDeviceAddress = VK_TRUE;
		vulkan12Features.separateDepthStencilLayouts = VK_TRUE;
		vulkan12Features.shaderSubgroupExtendedTypes = VK_TRUE;
		vulkan12Features.uniformBufferStandardLayout = VK_TRUE;
		vulkan12Features.scalarBlockLayout = VK_TRUE;
		vulkan12Features.shaderFloat16 = VK_TRUE;
		vulkan12Features.drawIndirectCount = VK_TRUE;
		vulkan12Features.bufferDeviceAddress = VK_TRUE;
		vulkan12Features.timelineSemaphore = VK_TRUE;
		ADD_P_NEXT(vulkan12Features);

		VkPhysicalDeviceVulkan11Features vulkan11Features{};
		vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		vulkan11Features.shaderDrawParameters = VK_TRUE;
		vulkan11Features.storageBuffer16BitAccess = VK_TRUE;
		ADD_P_NEXT(vulkan11Features);

		VkPhysicalDeviceFeatures2 features{};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.features.robustBufferAccess = VK_TRUE;
		features.features.shaderInt64 = VK_TRUE;
		features.features.shaderInt16 = VK_TRUE;
		features.features.textureCompressionBC = VK_TRUE;
		features.features.fragmentStoresAndAtomics = VK_TRUE;
		features.features.vertexPipelineStoresAndAtomics = VK_TRUE;
		features.features.imageCubeArray = VK_TRUE;
		features.features.multiViewport = VK_TRUE;
		features.features.samplerAnisotropy = VK_TRUE;
		features.features.shaderStorageImageReadWithoutFormat = VK_TRUE;
		features.features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
		features.features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
		features.features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
		features.features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
		features.features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
		features.features.depthClamp = VK_TRUE;
		features.features.depthBiasClamp = VK_TRUE;
		features.features.depthBounds = VK_TRUE;
		ADD_P_NEXT(features);
		#undef ADD_P_NEXT

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &features;
		deviceCreateInfo.enabledExtensionCount = deviceExtensions.GetCount();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.GetData();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.GetData();
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.GetCount();

		const auto result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
		if (result != VK_SUCCESS) {
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VkDevice, VkResult({})", (i32)result);
			return false;
		}

		// get queues
		{
			vkGetDeviceQueue(m_device,
				m_queueFamilies[static_cast<u32>(QueueType::Graphics)], 0, &m_queues[static_cast<u32>(QueueType::Graphics)]);

			// if compute queue has own family
			if (m_queueFamilies[static_cast<u32>(QueueType::Compute)] != m_queueFamilies[static_cast<u32>(QueueType::Graphics)]) {
				vkGetDeviceQueue(m_device,
					m_queueFamilies[static_cast<u32>(QueueType::Compute)], 0, &m_queues[static_cast<u32>(QueueType::Compute)]);
				Terminal::Info(StringHelper::GetTypeName(this), "Compute queue uses own family, index: {} ", m_queueFamilies[static_cast<u32>(QueueType::Compute)]);
			}
			else if (queueCreateInfos[0].queueCount > 1) {
				vkGetDeviceQueue(m_device,
					m_queueFamilies[static_cast<u32>(QueueType::Compute)], 1, &m_queues[static_cast<u32>(QueueType::Compute)]);
				Terminal::Info(StringHelper::GetTypeName(this), "Compute queue uses graphics queue family");
			}
			else {
				m_queues[static_cast<u32>(QueueType::Compute)] = m_queues[static_cast<u32>(QueueType::Graphics)];
				Terminal::Info(StringHelper::GetTypeName(this), "Compute queue same with graphics queue");
			}

			if (m_queueFamilies[static_cast<u32>(QueueType::Transfer)] == m_queueFamilies[static_cast<u32>(QueueType::Graphics)]) {
				if (queueCreateInfos[0].queueCount > 1) {
					vkGetDeviceQueue(m_device,
						m_queueFamilies[static_cast<u32>(QueueType::Graphics)], 1, &m_queues[static_cast<u32>(QueueType::Transfer)]);
					Terminal::Info(StringHelper::GetTypeName(this), "Transfer queue uses graphics queue family");
				}
				else {
					m_queues[static_cast<u32>(QueueType::Transfer)] = m_queues[static_cast<u32>(QueueType::Graphics)];
					Terminal::Info(StringHelper::GetTypeName(this), "Transfer queue same with graphics queue");
				}
			}
			else if (m_queueFamilies[static_cast<u32>(QueueType::Transfer)] == m_queueFamilies[static_cast<u32>(QueueType::Compute)]) {
				if (queueCreateInfos[1].queueCount > 1) {
					vkGetDeviceQueue(m_device,
						m_queueFamilies[static_cast<u32>(QueueType::Compute)], 1, &m_queues[static_cast<u32>(QueueType::Transfer)]);
					Terminal::Info(StringHelper::GetTypeName(this), "Transfer queue uses compute queue family");
				}
				else {
					m_queues[static_cast<u32>(QueueType::Transfer)] = m_queues[static_cast<u32>(QueueType::Compute)];
					Terminal::Info(StringHelper::GetTypeName(this), "Transfer queue same with compute queue");
				}
			}
			else /* transfer queue has own family */ {
				vkGetDeviceQueue(m_device,
					m_queueFamilies[static_cast<u32>(QueueType::Transfer)], 0, &m_queues[static_cast<u32>(QueueType::Transfer)]);
				Terminal::Info(StringHelper::GetTypeName(this), "Transfer queue uses own family, index: {} ", m_queueFamilies[static_cast<u32>(QueueType::Transfer)]);
			}
		}

		VkPhysicalDeviceDescriptorHeapPropertiesEXT descriptorHeapProperties{};
		descriptorHeapProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT;

		VkPhysicalDeviceProperties2 deviceProperties2{};
		deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProperties2.pNext = &descriptorHeapProperties;

		vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties2);

		m_resourceDescriptorSize = std::max(descriptorHeapProperties.imageDescriptorSize, descriptorHeapProperties.bufferDescriptorSize);
		m_samplerDescriptorSize = descriptorHeapProperties.samplerDescriptorSize;

		m_resourceHeapReservedRange = descriptorHeapProperties.minResourceHeapReservedRange;
		m_samplerHeapReservedRange = descriptorHeapProperties.minSamplerHeapReservedRange;

		return true;
	}

	b8 VulkanDevice::CreateVMAAllocator()
	{
		VmaVulkanFunctions vulkanFunctions{};
		vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo createInfo{};
		createInfo.physicalDevice = m_physicalDevice;
		createInfo.device = m_device;
		createInfo.instance = m_instance;
		createInfo.flags
			= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
			| VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT
			| VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT
			| VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
			;

		createInfo.vulkanApiVersion = VK_API_VERSION_1_4;
		createInfo.pVulkanFunctions = &vulkanFunctions;

		const auto result = vmaCreateAllocator(&createInfo, &m_allocator);
		if (result != VK_SUCCESS) {
			Terminal::Error(StringHelper::GetTypeName(this), "Failed to create VMA allocator, VkResult({})", (i32)result);
			return false;
		}

		return true;
	}

	void VulkanDevice::InitializeImGui(u32 framesInFlight, DescriptorHeap* pResourceHeap, TextureFormat colorFormat) 
	{
		const VkFormat vkColorFormat = Helpers::ToFormat(colorFormat);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.UseDynamicRendering = true;
		initInfo.ApiVersion = VK_API_VERSION_1_4;
		initInfo.Instance = m_instance;
		initInfo.PhysicalDevice = m_physicalDevice;
		initInfo.Device = m_device;
		initInfo.QueueFamily = m_queueFamilies[static_cast<u32>(QueueType::Graphics)];
		initInfo.Queue = m_queues[static_cast<u32>(QueueType::Graphics)];
		initInfo.DescriptorPoolSize = 4096;
		initInfo.MinImageCount = framesInFlight;
		initInfo.ImageCount = framesInFlight;
		initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vkColorFormat;
		initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		initInfo.PipelineInfoForViewports.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &vkColorFormat;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

		initInfo.CheckVkResultFn = [](VkResult result) {
			if (result != VK_SUCCESS) {
				Terminal::Error("ImGui", "Vulkan error: VkResult({})", (i32)result);
			}
		};

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
		io.BackendFlags = ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;

		ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
		pio.Platform_CreateVkSurface = [](ImGuiViewport* vp, ImU64 vk_instance,
										const void* vk_allocator, ImU64* out_vk_surface) -> int
		{
			VkWin32SurfaceCreateInfoKHR info = {};
			info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			info.hinstance = GetModuleHandle(nullptr);
			info.hwnd = (HWND)vp->PlatformHandleRaw;
			return (int)vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &info,
												(const VkAllocationCallbacks*)vk_allocator,
												(VkSurfaceKHR*)out_vk_surface);
		};

		ImGui_ImplVulkan_Init(&initInfo);
	}
	u64 VulkanDevice::RegisterImGuiTexture(TextureView* pView, Sampler* pSampler)
	{
		if (!pView)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ImGui texture could not be registered, view or sampler is missing");
			return 0;
		}

		VkImageView view = static_cast<VulkanTextureView*>(pView)->GetHandle();

		VkDescriptorSet set = ImGui_ImplVulkan_AddTexture(nullptr, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return reinterpret_cast<u64>(set);
	}

	void VulkanDevice::UnregisterImGuiTexture(u64 textureId)
	{
		if (textureId == 0)
			return;

		ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureId));
	}
	void VulkanDevice::NewFrameImGui() 
	{
		ImGui_ImplVulkan_NewFrame();
	}
	void VulkanDevice::ShutdownImGui() 
	{
		ImGui_ImplVulkan_Shutdown();
	}
}
