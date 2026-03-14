#include "GfxVkRenderPass.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkPipelineUtils.h>

#include <vector>

namespace Horizon
{
	GfxVkRenderPass::GfxVkRenderPass(const GfxRenderPassDesc& desc, GfxDevice* pDevice) : GfxRenderPass(desc, pDevice)
	{
        std::vector<VkAttachmentDescription> attachmentDescs;
        for (const auto& att : desc.attachments)
        {
            VkAttachmentDescription vkAtt{};
            vkAtt.format = VkImageUtils::GetVkImgFormat(att.format);
            vkAtt.samples = VK_SAMPLE_COUNT_1_BIT;
            vkAtt.loadOp = VkPipelineUtils::GetVkLoadOp(att.loadOp);
            vkAtt.storeOp = VkPipelineUtils::GetVkStoreOp(att.storeOp);
            vkAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            vkAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            vkAtt.initialLayout = VkPipelineUtils::GetVkImgLayout(att.initialLayout);
            vkAtt.finalLayout = VkPipelineUtils::GetVkImgLayout(att.finalLayout);
            attachmentDescs.push_back(vkAtt);
        }

        std::vector<VkSubpassDescription> vkSubpasses;
        std::vector<VkAttachmentReference> colorRefs;
        std::vector<VkAttachmentReference> depthRefs;
        std::vector<VkAttachmentReference> inputRefs;

        for (const auto& subpass : desc.subpasses)
        {
            VkSubpassDescription vkSubpass{};
            vkSubpass.pipelineBindPoint = VkPipelineUtils::GetVkBindPoint(subpass.bindPoint);
            if (subpass.colorAttachmentIndices.size() > 0)
            {
                colorRefs.resize(subpass.colorAttachmentIndices.size());
                for (u32 i = 0; i < subpass.colorAttachmentIndices.size(); i++)
                {
                    colorRefs[i].attachment = subpass.colorAttachmentIndices[i];
                    colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                vkSubpass.colorAttachmentCount = static_cast<u32>(colorRefs.size());
                vkSubpass.pColorAttachments = colorRefs.data();
            }
            if (subpass.depthAttachmentIndex != u32_max)
            {
                depthRefs.resize(1);
                depthRefs[0].attachment = subpass.depthAttachmentIndex;
                depthRefs[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                vkSubpass.pDepthStencilAttachment = &depthRefs[0];
            }
            if (subpass.inputAttachmentIndices.size() > 0)
            {
                inputRefs.resize(subpass.inputAttachmentIndices.size());
                for (u32 i = 0; i < subpass.inputAttachmentIndices.size(); i++)
                {
                    inputRefs[i].attachment = subpass.inputAttachmentIndices[i];
                    inputRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
                vkSubpass.inputAttachmentCount = static_cast<u32>(inputRefs.size());
                vkSubpass.pInputAttachments = inputRefs.data();
            }
            vkSubpasses.push_back(vkSubpass);
        }

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = dependency.srcStageMask;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        info.attachmentCount = static_cast<u32>(attachmentDescs.size());
        info.pAttachments = attachmentDescs.data();
        info.subpassCount = static_cast<u32>(vkSubpasses.size());
        info.pSubpasses = vkSubpasses.data();
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        VDebug::VkAssert(vkCreateRenderPass(VkDevice(Root()->Device()), &info, nullptr, &m_pass), "GfxVkRenderPass");
	}

	GfxVkRenderPass::~GfxVkRenderPass()
	{
        if (m_pass)
        {
            vkDestroyRenderPass(VkDevice(Root()->Device()), m_pass, nullptr);
            m_pass = VK_NULL_HANDLE;
        }
	}

	void* GfxVkRenderPass::Pass() const { return static_cast<void*>(m_pass); }
}
