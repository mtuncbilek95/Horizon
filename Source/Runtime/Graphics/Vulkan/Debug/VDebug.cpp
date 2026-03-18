#include "VDebug.h"

#define VMA_IMPLEMENTATION
#if defined(HORIZON_LINUX)
#include <vk_mem_alloc.h>
#elif defined(HORIZON_WINDOWS)
#include <vma/vk_mem_alloc.h>
#endif

#include <magic_enum/magic_enum.hpp>

#include <string>

namespace Horizon
{
    void VDebug::VkAssert(VkResult result, const std::string& objectTitle)
    {
        if (result != VK_SUCCESS && result != VK_ERROR_DEVICE_LOST && result != VK_ERROR_INITIALIZATION_FAILED)
            Log::Terminal(LogType::Error, "Vulkan::Assert", "{0} - {1}", objectTitle, magic_enum::enum_name<VkResult>(result));

        if (result == VK_ERROR_DEVICE_LOST || result == VK_ERROR_INITIALIZATION_FAILED)
            Log::Terminal(LogType::Error, "Vulkan::Assert", "{0} - {1}", objectTitle, magic_enum::enum_name<VkResult>(result));
    }
}
