#include "GfxVkInstance.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Horizon
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Log::Terminal(LogType::Info, "Vulkan::Validation", "{}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Log::Terminal(LogType::Warning, "Vulkan::Validation", "{}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Log::Terminal(LogType::Error, "Vulkan::Validation", "{}", pCallbackData->pMessage);
            break;
        default:
            break;
        }

        return false;
    }

    GfxVkInstance::GfxVkInstance(const GfxInstanceDesc& desc) : GfxInstance(desc), m_instance(VK_NULL_HANDLE),
        m_physicalDevice(VK_NULL_HANDLE)
    {
        VDebug::VkAssert(volkInitialize(), "GfxVkInstance");

#if defined(HORIZON_DEBUG)
        m_debugMessenger = VK_NULL_HANDLE;
#endif

        struct ExtensionEntry
        {
            const char* name;
            b8 supported;
        };

        std::vector<ExtensionEntry> extensions;
        std::vector<const char*> workingExtensions;
        extensions.push_back({ VK_KHR_SURFACE_EXTENSION_NAME, false });
        extensions.push_back({ VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, false });
        extensions.push_back({ VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, false });

#if defined(HORIZON_WINDOWS)
        extensions.push_back({ VK_KHR_WIN32_SURFACE_EXTENSION_NAME, false });
#endif

#if defined(HORIZON_LINUX)
        extensions.push_back({ VK_KHR_XCB_SURFACE_EXTENSION_NAME, false });
#endif

#if defined(HORIZON_DEBUG)
        extensions.push_back({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false });
        extensions.push_back({ VK_EXT_DEBUG_REPORT_EXTENSION_NAME, false });
#endif

        u32 extensionCount = 0;
        VDebug::VkAssert(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr), "GfxVkInstance");

        std::vector<VkExtensionProperties> allExtensions(extensionCount);
        VDebug::VkAssert(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, allExtensions.data()), "GfxVkInstance");

        for (usize i = 0; i < extensions.size(); ++i)
        {
            for (auto& extension : allExtensions)
            {
                if (strcmp(extensions[i].name, extension.extensionName) == 0)
                {
                    extensions[i].supported = true;
                    workingExtensions.push_back(extensions[i].name);
                    break;
                }
            }
        }

        // Print unsupported extensions
        for (auto& extension : extensions)
        {
            if (!extension.supported)
                Log::Terminal(LogType::Warning, "GfxVkInstance::GfxVkInstance", "Extension not supported: {}", extension.name);
        }

        u32 layerCount = 0;
        VDebug::VkAssert(vkEnumerateInstanceLayerProperties(&layerCount, nullptr), "GfxVkInstance");
        std::vector<VkLayerProperties> allLayers(layerCount);
        VDebug::VkAssert(vkEnumerateInstanceLayerProperties(&layerCount, allLayers.data()), "GfxVkInstance");

        std::vector<ExtensionEntry> wantedLayers;
        std::vector<const char*> workingLayers;
#if defined(HORIZON_DEBUG)
        wantedLayers.push_back({ "VK_LAYER_KHRONOS_validation", false });
        wantedLayers.push_back({ "VK_LAYER_LUNARG_screenshot" , false });
        wantedLayers.push_back({ "VK_LAYER_LUNARG_monitor", false });
#endif

        for (usize i = 0; i < wantedLayers.size(); ++i)
        {
            for (auto& layer : allLayers)
            {
                if (strcmp(wantedLayers[i].name, layer.layerName) == 0)
                {
                    wantedLayers[i].supported = true;
                    workingLayers.push_back(wantedLayers[i].name);
                    break;
                }
            }
        }

        // Print unsupported layers
        for (auto& layer : wantedLayers)
        {
            if (!layer.supported)
                Log::Terminal(LogType::Warning, "GfxVkInstance::GfxVkInstance", "Layer not supported: {}", layer.name);
        }

#if defined(HORIZON_DEBUG)
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        debugCreateInfo.pUserData = nullptr;
#endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = desc.appName.data();
        appInfo.applicationVersion = VK_MAKE_VERSION(desc.appVer.x, desc.appVer.y, desc.appVer.z);
        appInfo.pEngineName = desc.appName.data();
        appInfo.engineVersion = VK_MAKE_VERSION(desc.appVer.x, desc.appVer.y, desc.appVer.z);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<u32>(workingExtensions.size());
        createInfo.ppEnabledExtensionNames = workingExtensions.data();
        createInfo.enabledLayerCount = static_cast<u32>(workingLayers.size());
        createInfo.ppEnabledLayerNames = workingLayers.data();
#if defined(HORIZON_DEBUG)
        createInfo.pNext = &debugCreateInfo;
#endif

        VDebug::VkAssert(vkCreateInstance(&createInfo, nullptr, &m_instance), "GfxVkInstance::GfxVkInstance");

        volkLoadInstance(m_instance);

#if defined(HORIZON_DEBUG)
        VDebug::VkAssert(vkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger), "GfxVkInstance::GfxVkInstance");
#endif

        u32 deviceCount = 0;
        VDebug::VkAssert(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr), "GfxVkInstance::GfxVkInstance");

        // Temporary struct to hold the device and its score
        std::unordered_map<std::string, std::pair<VkPhysicalDevice, u32>> allDevices;

        // Get the physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VDebug::VkAssert(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()), "GfxVkInstance::GfxVkInstance");

        for (auto& device : devices)
        {
            // Get the device properties
            VkPhysicalDeviceProperties deviceProperties = {};
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            // Get the device features
            VkPhysicalDeviceFeatures deviceFeatures = {};
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            // Get the device memory properties
            VkPhysicalDeviceMemoryProperties deviceMemoryProperties = {};
            vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

            // Get the device queue family properties
            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

            VkPhysicalDeviceMeshShaderFeaturesEXT meshFeatures = {};
            meshFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

            VkPhysicalDeviceDescriptorBufferFeaturesEXT descBufFeatures = {};
            descBufFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
            descBufFeatures.pNext = &meshFeatures;

            VkPhysicalDeviceFeatures2 features2 = {};
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &descBufFeatures;

            vkGetPhysicalDeviceFeatures2(device, &features2);

            if (meshFeatures.meshShader && meshFeatures.taskShader)
                allDevices[deviceProperties.deviceName].second += 500;
            else
                continue;

            // Hold device on allDevices
            allDevices[deviceProperties.deviceName] = { device, 0 };

            for (auto& queueFamily : queueFamilyProperties)
                allDevices[deviceProperties.deviceName].second += queueFamily.queueCount;

            // Check if the device is discrete
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                allDevices[deviceProperties.deviceName].second += 1000;
            else
                allDevices[deviceProperties.deviceName].second += 100;

            // Check if the device supports geometry shaders
            if (deviceFeatures.geometryShader)
                allDevices[deviceProperties.deviceName].second += 10;
        }

        // Get the best device
        auto bestDevice = std::max_element(allDevices.begin(), allDevices.end(),
            [](const std::pair<std::string, std::pair<VkPhysicalDevice, u32>>& a, const std::pair<std::string, std::pair<VkPhysicalDevice, u32>>& b)
            {
                return a.second.second < b.second.second;
            });

        Log::Terminal(LogType::Success, "GfxVkInstance::GfxVkInstance", "Best device found: {}", bestDevice->first.c_str());
        m_physicalDevice = bestDevice->second.first;
    }

    GfxVkInstance::~GfxVkInstance()
    {
#if defined(HORIZON_DEBUG)
        if (m_debugMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
            m_debugMessenger = VK_NULL_HANDLE;
        }
#endif

        if (m_instance)
        {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
    }

    std::shared_ptr<GfxDevice> GfxVkInstance::CreateDevice(const GfxDeviceDesc& desc)
    {
        return std::make_shared<GfxVkDevice>(desc, this);
    }

    void* GfxVkInstance::Instance() const { return static_cast<void*>(m_instance); }
    void* GfxVkInstance::Adapter() const { return static_cast<void*>(m_physicalDevice); }
}
