#include "GfxVkDevice.h"

#include <Runtime/Graphics/Vulkan/Buffer/GfxVkBuffer.h>
#include <Runtime/Graphics/Vulkan/Command/GfxVkCommandBuffer.h>
#include <Runtime/Graphics/Vulkan/Command/GfxVkCommandPool.h>
#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorLayout.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorPool.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorSet.h>
#include <Runtime/Graphics/Vulkan/Descriptor/GfxVkDescriptorBuffer.h>
#include <Runtime/Graphics/Vulkan/Framebuffer/GfxVkFramebuffer.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImage.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImageView.h>
#include <Runtime/Graphics/Vulkan/Instance/GfxVkInstance.h>
#include <Runtime/Graphics/Vulkan/Pipeline/GfxVkPipeline.h>
#include <Runtime/Graphics/Vulkan/Queue/GfxVkQueue.h>
#include <Runtime/Graphics/Vulkan/RenderPass/GfxVkRenderPass.h>
#include <Runtime/Graphics/Vulkan/Sampler/GfxVkSampler.h>
#include <Runtime/Graphics/Vulkan/Shader/GfxVkShader.h>
#include <Runtime/Graphics/Vulkan/Swapchain/GfxVkSwapchain.h>
#include <Runtime/Graphics/Vulkan/Sync/GfxVkFence.h>
#include <Runtime/Graphics/Vulkan/Sync/GfxVkSemaphore.h>

#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>

#include <memory>
#include <vector>

namespace Horizon
{
    VkQueue GfxVkDevice::QueueFamily::GetFreeQueue()
    {
        if (m_queueCount > 0)
        {
            VkQueue queue = m_queues.back();
            m_queues.pop_back();
            m_queueCount--;
            return queue;
        }

        return VK_NULL_HANDLE;
    }

    void GfxVkDevice::QueueFamily::FillQueues(VkDevice device)
    {
        for (u32 i = 0; i < m_requestedCount; i++)
        {
            VkQueue queue;
            vkGetDeviceQueue(device, m_familyIndex, i, &queue);
            m_queues.push_back(queue);
        }
    }

    GfxVkDevice::GfxVkDevice(const GfxDeviceDesc& desc, GfxInstance* pInstance) : GfxDevice(desc, pInstance), 
		m_subgroupProps(), m_rayTracingProps()
    {
        m_instance = static_cast<VkInstance>(pInstance->Instance());
        m_adapter = static_cast<VkPhysicalDevice>(pInstance->Adapter());

        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_adapter, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_adapter, &queueFamilyCount, queueFamilies.data());

        i32 index = 0;
        for (auto& prop : queueFamilies)
        {
            if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT && m_graphicsQueue.m_familyIndex == 255)
            {
                m_graphicsQueue.m_familyIndex = index;
                m_graphicsQueue.m_queueCount = prop.queueCount;
                m_graphicsQueue.m_requestedCount = m_graphicsQueue.m_requestedCount > prop.queueCount ? prop.queueCount :
                    desc.graphicsQueueCount;
            }
            else if (prop.queueFlags & VK_QUEUE_COMPUTE_BIT && m_computeQueue.m_familyIndex == 255)
            {
                m_computeQueue.m_familyIndex = index;
                m_computeQueue.m_queueCount = prop.queueCount;
                m_computeQueue.m_requestedCount = m_computeQueue.m_requestedCount > prop.queueCount ? prop.queueCount :
                    desc.computeQueueCount;
            }
            else if (prop.queueFlags & VK_QUEUE_TRANSFER_BIT && m_transferQueue.m_familyIndex == 255)
            {
                m_transferQueue.m_familyIndex = index;
                m_transferQueue.m_queueCount = prop.queueCount;
                m_transferQueue.m_requestedCount = m_transferQueue.m_requestedCount > prop.queueCount ? prop.queueCount :
                    desc.transferQueueCount;
            }
            index++;
        }

        std::vector<f32> queuePriorities(16, 1.0f);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        if (m_graphicsQueue.m_familyIndex != 255)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_graphicsQueue.m_familyIndex;
            queueCreateInfo.queueCount = m_graphicsQueue.m_requestedCount;
            queueCreateInfo.pQueuePriorities = queuePriorities.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        if (m_computeQueue.m_familyIndex != 255 && m_computeQueue.m_requestedCount > 0)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_computeQueue.m_familyIndex;
            queueCreateInfo.queueCount = m_computeQueue.m_requestedCount;
            queueCreateInfo.pQueuePriorities = queuePriorities.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        if (m_transferQueue.m_familyIndex != 255 && m_transferQueue.m_requestedCount > 0)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_transferQueue.m_familyIndex;
            queueCreateInfo.queueCount = m_transferQueue.m_requestedCount;
            queueCreateInfo.pQueuePriorities = queuePriorities.data();
            queueCreateInfos.push_back(queueCreateInfo);
        }

        struct ExtensionEntry
        {
            const char* name;
            b8 supported;
        };

        std::vector<ExtensionEntry> extensions;
        std::vector<const char*> workingExtensions;
        extensions.push_back({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, false });
        extensions.push_back({ VK_KHR_MAINTENANCE3_EXTENSION_NAME, false });
        extensions.push_back({ VK_KHR_MAINTENANCE_5_EXTENSION_NAME, false });
        extensions.push_back({ VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, false });
        extensions.push_back({ VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME, false });
        extensions.push_back({ VK_EXT_MESH_SHADER_EXTENSION_NAME, false });
		extensions.push_back({ VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME, false });
		extensions.push_back({ VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, false });

        //Check if the device supports the extensions
        u32 extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_adapter, nullptr, &extensionCount, availableExtensions.data());

        for (usize i = 0; i < extensions.size(); ++i)
        {
            for (auto& extension : availableExtensions)
            {
                if (strcmp(extensions[i].name, extension.extensionName) == 0)
                {
                    extensions[i].supported = true;
                    workingExtensions.push_back(extensions[i].name);
                    break;
                }
            }
        }

        for (auto& extension : extensions)
            if (extension.supported)
                workingExtensions.push_back(extension.name);

        VkPhysicalDeviceMaintenance4Features maintenance4Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES };
        maintenance4Features.maintenance4 = VK_TRUE;

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddress = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
        bufferDeviceAddress.bufferDeviceAddress = VK_TRUE;
        bufferDeviceAddress.pNext = &maintenance4Features;

        VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
        meshFeatures.taskShader = VK_TRUE;
        meshFeatures.meshShader = VK_TRUE;
        meshFeatures.multiviewMeshShader = VK_FALSE;
        meshFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        meshFeatures.meshShaderQueries = VK_FALSE;
        meshFeatures.pNext = &bufferDeviceAddress;

        VkPhysicalDeviceDescriptorBufferFeaturesEXT descBufFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT };
        descBufFeatures.descriptorBuffer = VK_TRUE;
        descBufFeatures.descriptorBufferPushDescriptors = VK_TRUE;
        descBufFeatures.descriptorBufferCaptureReplay = VK_TRUE;
        descBufFeatures.pNext = &meshFeatures;

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
        dynamicRendering.dynamicRendering = VK_TRUE;
        dynamicRendering.pNext = &descBufFeatures;

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR };
        maintenance5Features.maintenance5 = VK_TRUE;
        maintenance5Features.pNext = &dynamicRendering;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        deviceFeatures2.pNext = &maintenance5Features;
        vkGetPhysicalDeviceFeatures2(m_adapter, &deviceFeatures2);

        meshFeatures.multiviewMeshShader = VK_FALSE;
        meshFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        meshFeatures.meshShaderQueries = VK_FALSE;

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
        deviceCreateInfo.enabledExtensionCount = static_cast<u32>(workingExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = workingExtensions.data();
        deviceCreateInfo.pNext = &deviceFeatures2;

        VDebug::VkAssert(vkCreateDevice(m_adapter, &deviceCreateInfo, nullptr, &m_device), "GfxVkDevice");
        volkLoadDevice(m_device);

        m_graphicsQueue.FillQueues(m_device);
        m_computeQueue.FillQueues(m_device);
        m_transferQueue.FillQueues(m_device);

        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(vkGetInstanceProcAddr);
        vulkanFunctions.vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(vkGetDeviceProcAddr);

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
        allocatorInfo.physicalDevice = m_adapter;
        allocatorInfo.device = m_device;
        allocatorInfo.instance = m_instance;
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        VDebug::VkAssert(vmaCreateAllocator(&allocatorInfo, &m_allocator), "GfxVkDevice::VMA");

        m_descBufferProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT };
        m_meshShaderProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
        m_deviceProps = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };

        m_deviceProps.pNext = &m_descBufferProps;
        m_descBufferProps.pNext = &m_meshShaderProps;

        vkGetPhysicalDeviceProperties2(m_adapter, &m_deviceProps);
    }

    GfxVkDevice::~GfxVkDevice()
    {
        if (m_allocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }

        if (m_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }
    }

    void* GfxVkDevice::Device() const { return static_cast<void*>(m_device); }
    void* GfxVkDevice::Instance() const { return static_cast<void*>(m_instance); }
    void* GfxVkDevice::Adapter() const { return static_cast<void*>(m_adapter); }
    void* GfxVkDevice::Allocator() const { return static_cast<void*>(m_allocator); }

    void GfxVkDevice::WaitIdle() const
    {
        vkDeviceWaitIdle(m_device);
    }

    std::shared_ptr<GfxQueue> GfxVkDevice::CreateQueue(QueueType type)
    {
        switch (type)
        {
        case QueueType::Graphics:
        {
            GfxQueueDesc desc = GfxQueueDesc().setType(type)
                .setIndex(m_graphicsQueue.m_familyIndex);

            std::shared_ptr<GfxVkQueue> q = std::make_shared<GfxVkQueue>(desc, this);
            q->m_queue = m_graphicsQueue.GetFreeQueue();
            return q;
        }
        case QueueType::Transfer:
        {
            GfxQueueDesc desc = GfxQueueDesc().setType(type)
                .setIndex(m_transferQueue.m_familyIndex);

            std::shared_ptr<GfxVkQueue> q = std::make_shared<GfxVkQueue>(desc, this);
            q->m_queue = m_transferQueue.GetFreeQueue();
            return q;
        }
        case QueueType::Compute:
        {
            GfxQueueDesc desc = GfxQueueDesc().setType(type)
                .setIndex(m_computeQueue.m_familyIndex);

            std::shared_ptr<GfxVkQueue> q = std::make_shared<GfxVkQueue>(desc, this);
            q->m_queue = m_computeQueue.GetFreeQueue();
            return q;
        }
        case QueueType::Sparse:
        {
            Log::Terminal(LogType::Error, "GfxVkDevice::CreateQueue", "SparseQueue has not been implemented yet!");
            return nullptr;
        }
        default:
            Log::Terminal(LogType::Error, "GfxVkDevice::CreateQueue", "Could not find the requested queue!");
            return nullptr;
        }

        return nullptr;
    }

    std::shared_ptr<GfxImage> GfxVkDevice::CreateImage(const GfxImageDesc& desc)
    {
        return std::make_shared<GfxVkImage>(desc, this);
    }

    std::shared_ptr<GfxImageView> GfxVkDevice::CreateView(const GfxImageViewDesc& desc)
    {
        return std::make_shared<GfxVkImageView>(desc, this);
    }

    std::shared_ptr<GfxSwapchain> GfxVkDevice::CreateSwapchain(const GfxSwapchainDesc& desc)
    {
        return std::make_shared<GfxVkSwapchain>(desc, this);
    }

    std::shared_ptr<GfxBuffer> GfxVkDevice::CreateBuffer(const GfxBufferDesc& desc)
    {
        return std::make_shared<GfxVkBuffer>(desc, this);
    }

    std::shared_ptr<GfxPipeline> GfxVkDevice::CreateGraphicsPipeline(const GfxGraphicsPipelineDesc& desc)
    {
        return std::make_shared<GfxVkPipeline>(desc, this);
    }

    std::shared_ptr<GfxPipeline> GfxVkDevice::CreateComputePipeline(const GfxComputePipelineDesc& desc)
    {
        return std::make_shared<GfxVkPipeline>(desc, this);
    }

    std::shared_ptr<GfxFence> GfxVkDevice::CreateSyncFence(const GfxFenceDesc& desc)
    {
        return std::make_shared<GfxVkFence>(desc, this);
    }

    std::shared_ptr<GfxSemaphore> GfxVkDevice::CreateSyncSemaphore(const GfxSemaphoreDesc& desc)
    {
        return std::make_shared<GfxVkSemaphore>(desc, this);
    }

    std::shared_ptr<GfxDescriptorLayout> GfxVkDevice::CreateDescriptorLayout(const GfxDescriptorLayoutDesc& desc)
    {
        return std::make_shared<GfxVkDescriptorLayout>(desc, this);
    }

    std::shared_ptr<GfxDescriptorPool> GfxVkDevice::CreateDescriptorPool(const GfxDescriptorPoolDesc& desc)
    {
        return std::make_shared<GfxVkDescriptorPool>(desc, this);
    }

    std::shared_ptr<GfxDescriptorSet> GfxVkDevice::CreateDescriptorSet(const GfxDescriptorSetDesc& desc)
    {
        return std::make_shared<GfxVkDescriptorSet>(desc, this);
    }

    std::shared_ptr<GfxDescriptorBuffer> GfxVkDevice::CreateDescriptorBuffer(const GfxDescriptorBufferDesc& desc)
    {
        return std::make_shared<GfxVkDescriptorBuffer>(desc, this);
    }

    std::shared_ptr<GfxShader> GfxVkDevice::CreateShader(const GfxShaderDesc& desc)
    {
        return std::make_shared<GfxVkShader>(desc, this);
    }

    std::shared_ptr<GfxCommandPool> GfxVkDevice::CreateCommandPool(const GfxCommandPoolDesc& desc)
    {
        return std::make_shared<GfxVkCommandPool>(desc, this);
    }

    std::shared_ptr<GfxCommandBuffer> GfxVkDevice::CreateCommandBuffer(const GfxCommandBufferDesc& desc)
    {
        return std::make_shared<GfxVkCommandBuffer>(desc, this);
    }

    std::shared_ptr<GfxFramebuffer> GfxVkDevice::CreateFramebuffer(const GfxFramebufferDesc& desc)
    {
        return std::make_shared<GfxVkFramebuffer>(desc, this);
    }

    std::shared_ptr<GfxRenderPass> GfxVkDevice::CreateRenderPass(const GfxRenderPassDesc& desc)
    {
        return std::make_shared<GfxVkRenderPass>(desc, this);
    }

    usize GfxVkDevice::DescTypeSize(DescriptorType type) const
    {
        switch (type)
        {
        case DescriptorType::Sampler:
            return m_descBufferProps.samplerDescriptorSize;
        case DescriptorType::CombinedSamplerImage:
            return m_descBufferProps.combinedImageSamplerDescriptorSize;
        case DescriptorType::SampledImage:
            return m_descBufferProps.sampledImageDescriptorSize;
        case DescriptorType::StorageImage:
            return m_descBufferProps.storageImageDescriptorSize;
        case DescriptorType::UniformTexelBuffer:
            return m_descBufferProps.uniformTexelBufferDescriptorSize;
        case DescriptorType::StorageTexelBuffer:
            return m_descBufferProps.storageTexelBufferDescriptorSize;
        case DescriptorType::Uniform:
            return m_descBufferProps.uniformBufferDescriptorSize;
        case DescriptorType::Storage:
            return m_descBufferProps.storageBufferDescriptorSize;
        case DescriptorType::DynamicUniform:
            return m_descBufferProps.uniformBufferDescriptorSize;
        case DescriptorType::DynamicStorage:
            return m_descBufferProps.storageBufferDescriptorSize;
        case DescriptorType::InputAttachment:
            return m_descBufferProps.inputAttachmentDescriptorSize;
        default:
            Log::Terminal(LogType::Error, "GfxVkDevice::DescTypeSize", "Could not find the requested descriptor type!");
            return 0;
        }
    }

    std::shared_ptr<GfxSampler> GfxVkDevice::CreateSampler(const GfxSamplerDesc& desc)
    {
        return std::make_shared<GfxVkSampler>(desc, this);
    }
}
