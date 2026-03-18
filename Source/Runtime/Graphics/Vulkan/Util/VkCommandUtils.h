#pragma once

#include <Runtime/Graphics/RHI/Util/CommandFormat.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
    struct VkCommandUtils
    {
        static VkCommandPoolCreateFlags GetVkPoolFlags(CommandPoolFlags flag);
        static VkCommandBufferLevel GetVkLevel(CommandLevel lvl);
        static VkCommandBufferUsageFlags GetVkCmdUsage(CommandBufferUsage usage);
        static VkSubpassContents GetVkContent(SubpassContent ctx);
    };
}
