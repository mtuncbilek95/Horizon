#pragma once

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

#include <string>

namespace Horizon
{
    struct VDebug final
    {
        static void VkAssert(VkResult result, const std::string& objectTitle);
    };
}
