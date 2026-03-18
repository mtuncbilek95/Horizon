#include "VkCommandUtils.h"

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{
    VkCommandPoolCreateFlags VkCommandUtils::GetVkPoolFlags(CommandPoolFlags flag)
    {
        VkCommandPoolCreateFlags flg = 0;

        for (auto u : magic_enum::enum_values<CommandPoolFlags>())
        {
            if (HasFlag(flag, u))
            {
                switch (u)
                {
                case CommandPoolFlags::ResetCommandBuffer:
                    flg |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    break;
                default:
                    break;
                }
            }
        }

        return flg;
    }

    VkCommandBufferLevel VkCommandUtils::GetVkLevel(CommandLevel lvl)
    {
        switch (lvl)
        {
        case CommandLevel::Primary:
            return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        case CommandLevel::Secondary:
            return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        default:
            return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        }
    }

    VkCommandBufferUsageFlags VkCommandUtils::GetVkCmdUsage(CommandBufferUsage usage)
    {
        VkCommandBufferUsageFlags flags = 0;

        for (auto u : magic_enum::enum_values<CommandBufferUsage>())
        {
            if (HasFlag(usage, u))
            {
                switch (u)
                {
                case CommandBufferUsage::OneTimeSubmit:
                    flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                    break;
                case CommandBufferUsage::RenderPassContinue:
                    flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                    break;
                case CommandBufferUsage::SimultaneousUse:
                    flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                    break;
                default:
                    break;
                }
            }
        }

        return flags;
    }

    VkSubpassContents VkCommandUtils::GetVkContent(SubpassContent ctx)
    {
        switch (ctx)
        {
        case SubpassContent::ContentsInline:
            return VK_SUBPASS_CONTENTS_INLINE;
        case SubpassContent::SecondaryBuffer:
            return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
        case SubpassContent::InlineAndSecondaryBuffer:
            return VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR;
        default:
            return VK_SUBPASS_CONTENTS_INLINE;
        }
    }
}
