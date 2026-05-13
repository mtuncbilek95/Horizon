#include "GfxVkCommandBuffer.h"

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorSet.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/Vulkan/Buffer/GfxVkBuffer.h>
#include <Runtime/Graphics/Vulkan/Command/GfxVkCommandPool.h>
#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Framebuffer/GfxVkFramebuffer.h>
#include <Runtime/Graphics/Vulkan/Queue/GfxVkQueue.h>
#include <Runtime/Graphics/Vulkan/RenderPass/GfxVkRenderPass.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkCommandUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkPipelineUtils.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>

#include <vector>

namespace Horizon
{
	GfxVkCommandBuffer::GfxVkCommandBuffer(const GfxCommandBufferDesc& desc, GfxDevice* pDevice) : GfxCommandBuffer(desc, pDevice)
	{
		VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandPool = VkCommandPool(desc.pool->Pool());
		allocInfo.level = VkCommandUtils::GetVkLevel(desc.level);
		allocInfo.commandBufferCount = 1;

		VDebug::VkAssert(vkAllocateCommandBuffers(VkDevice(Root()->Device()), &allocInfo, &m_buffer), "GfxVkCommandBuffer");
	}

	GfxVkCommandBuffer::~GfxVkCommandBuffer()
	{
		if (m_buffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(VkDevice(Root()->Device()), VkCommandPool(Pool()->Pool()), 1, &m_buffer);
			m_buffer = VK_NULL_HANDLE;
		}
	}

	void* GfxVkCommandBuffer::Buffer() const { return static_cast<void*>(m_buffer); }

	void GfxVkCommandBuffer::BeginRecord(CommandBufferUsage usage) const
	{
		vkResetCommandBuffer(m_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		info.flags = VkCommandUtils::GetVkCmdUsage(usage);
		VDebug::VkAssert(vkBeginCommandBuffer(m_buffer, &info), "GfxVkCommandBuffer::BeginRecord");
	}

	void GfxVkCommandBuffer::BeginRecord(const InheritanceDesc& desc, CommandBufferUsage usage) const
	{
		vkResetCommandBuffer(m_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		info.flags = VkCommandUtils::GetVkCmdUsage(usage);

		VkCommandBufferInheritanceInfo inheritInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
		if (info.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT || info.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
		{
			inheritInfo.renderPass = VkRenderPass(desc.renderPass->Pass());
			inheritInfo.framebuffer = desc.framebuffer ? VkFramebuffer(desc.framebuffer->Framebuffer(desc.frameIndex)) : VK_NULL_HANDLE;
			inheritInfo.subpass = desc.subpass;
			inheritInfo.occlusionQueryEnable = false;

			info.pInheritanceInfo = &inheritInfo;
		}

		VDebug::VkAssert(vkBeginCommandBuffer(m_buffer, &info), "GfxVkCommandBuffer::BeginRecord_Inheritance");
	}

	void GfxVkCommandBuffer::BeginRecord(const DynamicInheritanceDesc& desc, CommandBufferUsage usage) const
	{
		vkResetCommandBuffer(m_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		info.flags = VkCommandUtils::GetVkCmdUsage(usage);

		std::vector<VkFormat> formats;
		VkCommandBufferInheritanceRenderingInfo inheritanceRendering = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
		VkCommandBufferInheritanceInfo inheritanceInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
		if (info.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT || info.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
		{
			for (auto f : desc.colorFormats)
				formats.push_back(VkImageUtils::GetVkImgFormat(f));

			inheritanceRendering.colorAttachmentCount = formats.size();
			inheritanceRendering.pColorAttachmentFormats = formats.data();
			inheritanceRendering.depthAttachmentFormat = VkImageUtils::GetVkImgFormat(desc.depthFormat);
			inheritanceRendering.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
			inheritanceRendering.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			inheritanceInfo.renderPass = VK_NULL_HANDLE;
			inheritanceInfo.subpass = 0;
			inheritanceInfo.framebuffer = VK_NULL_HANDLE;
			inheritanceInfo.pNext = &inheritanceRendering;

			info.pInheritanceInfo = &inheritanceInfo;
		}

		VDebug::VkAssert(vkBeginCommandBuffer(m_buffer, &info), "GfxVkCommandBuffer::BeginRecord_Inheritance");
	}

	void GfxVkCommandBuffer::EndRecord() const
	{
		vkEndCommandBuffer(m_buffer);
	}

	void GfxVkCommandBuffer::BeginRendering(const RenderingInfo& info)
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachments(info.colorAttachments.size());
		for (usize i = 0; i < info.colorAttachments.size(); i++)
		{
			const auto& src = info.colorAttachments[i];
			colorAttachments[i] = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
			colorAttachments[i].imageView = src.imageView ? VkImageView(src.imageView->View()) : VK_NULL_HANDLE;
			colorAttachments[i].imageLayout = VkPipelineUtils::GetVkImgLayout(src.imageLayout);
			colorAttachments[i].resolveMode = VK_RESOLVE_MODE_NONE;
			colorAttachments[i].resolveImageView = VK_NULL_HANDLE;
			colorAttachments[i].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachments[i].loadOp = VkPipelineUtils::GetVkLoadOp(src.loadOp);
			colorAttachments[i].storeOp = VkPipelineUtils::GetVkStoreOp(src.storeOp);
			colorAttachments[i].clearValue.color = { src.clearValue.color.r, src.clearValue.color.g, src.clearValue.color.b, src.clearValue.color.a };
		}

		VkRenderingAttachmentInfo depthAttachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		if (info.depthAttachment)
		{
			const auto& src = *info.depthAttachment;
			depthAttachment.imageView = src.imageView ? VkImageView(src.imageView->View()) : VK_NULL_HANDLE;
			depthAttachment.imageLayout = VkPipelineUtils::GetVkImgLayout(src.imageLayout);
			depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
			depthAttachment.resolveImageView = VK_NULL_HANDLE;
			depthAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.loadOp = VkPipelineUtils::GetVkLoadOp(src.loadOp);
			depthAttachment.storeOp = VkPipelineUtils::GetVkStoreOp(src.storeOp);
			depthAttachment.clearValue.depthStencil = { src.clearValue.depth, src.clearValue.stencil };
		}

		VkRenderingAttachmentInfo stencilAttachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		if (info.stencilAttachment)
		{
			const auto& src = *info.stencilAttachment;
			stencilAttachment.imageView = src.imageView ? VkImageView(src.imageView->View()) : VK_NULL_HANDLE;
			stencilAttachment.imageLayout = VkPipelineUtils::GetVkImgLayout(src.imageLayout);
			stencilAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
			stencilAttachment.resolveImageView = VK_NULL_HANDLE;
			stencilAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			stencilAttachment.loadOp = VkPipelineUtils::GetVkLoadOp(src.loadOp);
			stencilAttachment.storeOp = VkPipelineUtils::GetVkStoreOp(src.storeOp);
			stencilAttachment.clearValue.depthStencil = { src.clearValue.depth, src.clearValue.stencil };
		}

		VkRenderingFlags vkFlags = 0;
		if (HasFlag(info.flags, RenderingFlags::ContentsSecondary))
			vkFlags |= VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
		if (HasFlag(info.flags, RenderingFlags::Suspending))
			vkFlags |= VK_RENDERING_SUSPENDING_BIT;
		if (HasFlag(info.flags, RenderingFlags::Resuming))
			vkFlags |= VK_RENDERING_RESUMING_BIT;

		VkRenderingInfo renderingInfo = { VK_STRUCTURE_TYPE_RENDERING_INFO };
		renderingInfo.flags = vkFlags;
		renderingInfo.renderArea = { { info.renderAreaOffset.x, info.renderAreaOffset.y }, { info.renderAreaExtent.x, info.renderAreaExtent.y } };
		renderingInfo.layerCount = info.layerCount;
		renderingInfo.viewMask = info.viewMask;
		renderingInfo.colorAttachmentCount = static_cast<u32>(colorAttachments.size());
		renderingInfo.pColorAttachments = colorAttachments.empty() ? nullptr : colorAttachments.data();
		renderingInfo.pDepthAttachment = info.depthAttachment ? &depthAttachment : nullptr;
		renderingInfo.pStencilAttachment = info.stencilAttachment ? &stencilAttachment : nullptr;

		vkCmdBeginRendering(m_buffer, &renderingInfo);
	}

	void GfxVkCommandBuffer::EndRendering()
	{
		vkCmdEndRendering(m_buffer);
	}

	void GfxVkCommandBuffer::BeginRenderPass(const BeginRenderDesc& desc) const
	{
		u32 count = 0;
		count += desc.clearColor ? 1 : 0;
		count += desc.clearDepth ? 1 : 0;

		std::vector<VkClearValue> clears(count);
		for (usize i = 0; i < clears.size(); i++)
		{
			if (i == 0)
				clears[i].color = { desc.colorVal.x, desc.colorVal.y , desc.colorVal.z, desc.colorVal.w };
			else
				clears[i].depthStencil = { desc.depthVal.x, static_cast<u32>(desc.depthVal.y) };
		}

		VkRenderPassBeginInfo beginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		beginInfo.renderPass = VkRenderPass(desc.renderPass->Pass());
		beginInfo.framebuffer = VkFramebuffer(desc.framebuffer->Framebuffer(desc.frameIndex));
		beginInfo.renderArea = { {}, { desc.renderArea.x , desc.renderArea.y } };
		beginInfo.clearValueCount = static_cast<u32>(clears.size());
		beginInfo.pClearValues = clears.data();

		vkCmdBeginRenderPass(m_buffer, &beginInfo, VkCommandUtils::GetVkContent(desc.content));
	}

	void GfxVkCommandBuffer::EndRenderPass() const
	{
		vkCmdEndRenderPass(m_buffer);
	}

	void GfxVkCommandBuffer::BindPipeline(GfxPipeline* pipeline) const
	{
		GfxCommandBuffer::BindPipeline(pipeline);

		VkPipelineBindPoint bp = pipeline->Type() == PipelineType::Compute
			? VK_PIPELINE_BIND_POINT_COMPUTE
			: VK_PIPELINE_BIND_POINT_GRAPHICS;
		vkCmdBindPipeline(m_buffer, bp, VkPipeline(pipeline->Pipeline()));
	}

	void GfxVkCommandBuffer::BindVertices(const std::vector<GfxBuffer*> buffers) const
	{
		std::vector<VkBuffer> bf(buffers.size());

		usize offset = 0;
		for (u32 i = 0; i < buffers.size(); i++)
			bf[i] = VkBuffer(buffers[i]->Buffer());

		vkCmdBindVertexBuffers(m_buffer, 0, bf.size(), bf.data(), &offset);
	}

	void GfxVkCommandBuffer::BindIndex(const GfxBuffer* buffer, usize offset) const
	{
		vkCmdBindIndexBuffer(m_buffer, VkBuffer(buffer->Buffer()), offset, VK_INDEX_TYPE_UINT32);
	}

	void GfxVkCommandBuffer::DrawVertexed(u32 vertexCount, u32 firstVertex, u32 firstInstance, u32 instanceCount) const
	{
		vkCmdDraw(m_buffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void GfxVkCommandBuffer::DrawIndexed(u32 indexCount, u32 indexOffset, u32 vertexOffset, u32 instanceOffset, u32 instanceCount) const
	{
		vkCmdDrawIndexed(m_buffer, indexCount, instanceCount, 0, vertexOffset, 0);
	}

	void GfxVkCommandBuffer::ExecuteCommands(const std::vector<GfxCommandBuffer*> buffers) const
	{
		if (buffers.size() <= 0)
			return;

		std::vector<VkCommandBuffer> cmds(buffers.size());
		for (u32 i = 0; i < cmds.size(); i++)
			cmds[i] = VkCommandBuffer(buffers[i]->Buffer());

		if (cmds.size() > 0)
			vkCmdExecuteCommands(m_buffer, cmds.size(), cmds.data());
	}

	void GfxVkCommandBuffer::DrawMeshTask(u32 groupCountX, u32 groupCountY, u32 groupCountZ) const
	{
		vkCmdDrawMeshTasksEXT(m_buffer, groupCountX, groupCountY, groupCountZ);
	}

	void GfxVkCommandBuffer::DispatchCompute(u32 x, u32 y, u32 z) const
	{
		vkCmdDispatch(m_buffer, x, y, z);
	}

	void GfxVkCommandBuffer::BindDescriptorBuffer(const GfxPipeline* pipeline, ShaderStage stage, u32 binding, GfxDescriptorBuffer* buffer) const
	{
		VkDescriptorBufferBindingInfoEXT bindingInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT };
		bindingInfo.address = buffer->DeviceAddress();
		bindingInfo.usage = VkDescriptorUtils::GetVkDescBufferUsage(buffer->Type());
		vkCmdBindDescriptorBuffersEXT(m_buffer, 1, &bindingInfo);

		u32 bufferIndex = 0;
		VkDeviceSize offset = 0;
		vkCmdSetDescriptorBufferOffsetsEXT(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(pipeline->PipelineLayout()), binding, 1, &bufferIndex, &offset);
	}

	void GfxVkCommandBuffer::BindDescriptorSet(const GfxPipeline* pipeline, const GfxDescriptorSet* set, u32 firstSet) const
	{
		VkPipelineBindPoint bp = pipeline->Type() == PipelineType::Compute
			? VK_PIPELINE_BIND_POINT_COMPUTE
			: VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkDescriptorSet ds = VkDescriptorSet(set->DescSet());
		vkCmdBindDescriptorSets(m_buffer, bp,
			VkPipelineLayout(pipeline->PipelineLayout()),
			firstSet, 1, &ds, 0, nullptr);
	}

	void GfxVkCommandBuffer::CopyStageToBuffer(const CopyBufferDesc& desc) const
	{
		VkBufferCopy copyInfo = {};
		copyInfo.srcOffset = desc.srcOffset;
		copyInfo.dstOffset = desc.dstOffset;
		copyInfo.size = desc.size;
		vkCmdCopyBuffer(m_buffer, VkBuffer(desc.src->Buffer()), VkBuffer(desc.dst->Buffer()), 1, &copyInfo);
	}

	void GfxVkCommandBuffer::BindPushConstants(ShaderStage stage, u32 offset, u32 size, const void* data) const
	{
		if (!m_boundPipeline)
			return;

		vkCmdPushConstants(m_buffer, VkPipelineLayout(m_boundPipeline->PipelineLayout()),
			VkPipelineUtils::GetShaderType(stage), offset, size, data);
	}

	void GfxVkCommandBuffer::BindViewport(const Math::Vec2f& xy, const Math::Vec2f& size, const Math::Vec2f& depth) const
	{
		VkViewport vp = {};
		vp.x = xy.x;
		vp.y = xy.y;
		vp.width = size.x;
		vp.height = size.y;
		vp.minDepth = depth.x;
		vp.maxDepth = depth.y;

		vkCmdSetViewport(m_buffer, 0, 1, &vp);
	}

	void GfxVkCommandBuffer::BindScissor(const Math::Vec2i& offset, const Math::Vec2u& extent) const
	{
		VkRect2D scissor = {};
		scissor.offset.x = offset.x;
		scissor.offset.y = offset.y;
		scissor.extent.width = extent.x;
		scissor.extent.height = extent.y;

		vkCmdSetScissor(m_buffer, 0, 1, &scissor);
	}

	void GfxVkCommandBuffer::SetCullMode(CullMode mode) const
	{
		vkCmdSetCullMode((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkCullMode(mode));
	}

	void GfxVkCommandBuffer::SetFrontFace(FaceOrientation orientation) const
	{
		vkCmdSetFrontFace((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkFrontFace(orientation));
	}

	void GfxVkCommandBuffer::SetPrimitiveTopology(PrimitiveTopology topology) const
	{
		vkCmdSetPrimitiveTopology((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkTopo(topology));
	}

	void GfxVkCommandBuffer::SetDepthTestEnable(b8 enable) const
	{
		vkCmdSetDepthTestEnable((VkCommandBuffer)Buffer(), enable);
	}

	void GfxVkCommandBuffer::SetDepthWriteEnable(b8 enable) const
	{
		vkCmdSetDepthWriteEnable((VkCommandBuffer)Buffer(), enable);
	}

	void GfxVkCommandBuffer::SetDepthCompareOp(CompareOp op) const
	{
		vkCmdSetDepthCompareOp((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkCompareOp(op));
	}

	void GfxVkCommandBuffer::SetDepthBoundsTestEnable(b8 enable) const
	{
		vkCmdSetDepthBoundsTestEnable((VkCommandBuffer)Buffer(), enable);
	}

	void GfxVkCommandBuffer::SetDepthBounds(f32 min, f32 max) const
	{
		vkCmdSetDepthBounds((VkCommandBuffer)Buffer(), min, max);
	}

	void GfxVkCommandBuffer::SetDepthBiasEnable(b8 enable) const
	{
		vkCmdSetDepthBiasEnable((VkCommandBuffer)Buffer(), enable);
	}

	void GfxVkCommandBuffer::SetPolygonMode(PolygonMode mode) const
	{
		vkCmdSetPolygonModeEXT((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkPolygonMode(mode));
	}

	void GfxVkCommandBuffer::SetLogicOpEnable(b8 enable) const
	{
		vkCmdSetLogicOpEnableEXT((VkCommandBuffer)Buffer(), enable);
	}

	void GfxVkCommandBuffer::SetLogicOp(LogicOp op) const
	{
		vkCmdSetLogicOpEXT((VkCommandBuffer)Buffer(), VkPipelineUtils::GetVkLogicOp(op));
	}

	void GfxVkCommandBuffer::SetColorBlendEnable(u32 attachment, b8 enable) const
	{
		VkBool32 value = enable;
		vkCmdSetColorBlendEnableEXT((VkCommandBuffer)Buffer(), attachment, 1, &value);
	}

	void GfxVkCommandBuffer::SetColorBlendEquation(u32 attachment, const BlendAttachment& equation) const
	{
		VkColorBlendEquationEXT eq = {
			.srcColorBlendFactor = VkPipelineUtils::GetVkBlendFactor(equation.srcColor),
			.dstColorBlendFactor = VkPipelineUtils::GetVkBlendFactor(equation.dstColor),
			.colorBlendOp = VkPipelineUtils::GetVkBlendOp(equation.colorOp),
			.srcAlphaBlendFactor = VkPipelineUtils::GetVkBlendFactor(equation.srcAlpha),
			.dstAlphaBlendFactor = VkPipelineUtils::GetVkBlendFactor(equation.dstAlpha),
			.alphaBlendOp = VkPipelineUtils::GetVkBlendOp(equation.alphaOp),
		};
		vkCmdSetColorBlendEquationEXT((VkCommandBuffer)Buffer(), attachment, 1, &eq);
	}

	void GfxVkCommandBuffer::SetColorWriteMask(u32 attachment, ColorComponent mask) const
	{
		VkColorComponentFlags flags = VkPipelineUtils::GetVkColorComponents(mask);
		vkCmdSetColorWriteMaskEXT((VkCommandBuffer)Buffer(), attachment, 1, &flags);
	}

	void GfxVkCommandBuffer::BlitImage(const BlitImageDesc& desc) const
	{
		VkImageBlit region = {};
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.srcOffsets[0] = { 0, 0, 0 };
		region.srcOffsets[1] = { static_cast<i32>(desc.srcSize.x), static_cast<i32>(desc.srcSize.y), 1 };

		region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.dstOffsets[0] = { 0, 0, 0 };
		region.dstOffsets[1] = { static_cast<i32>(desc.dstSize.x), static_cast<i32>(desc.dstSize.y), 1 };

		vkCmdBlitImage(m_buffer,
			VkImage(desc.srcImage->Image()), VkPipelineUtils::GetVkImgLayout(desc.srcLayout),
			VkImage(desc.dstImage->Image()), VkPipelineUtils::GetVkImgLayout(desc.dstLayout),
			1, &region, VK_FILTER_LINEAR);
	}

	void GfxVkCommandBuffer::ImageBarrier(const ImageBarrierDesc& desc) const
	{
		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.srcAccessMask = VkPipelineUtils::GetVkAccessFlags(desc.srcAccess);
		barrier.dstAccessMask = VkPipelineUtils::GetVkAccessFlags(desc.dstAccess);;
		barrier.oldLayout = VkPipelineUtils::GetVkImgLayout(desc.oldLayout);
		barrier.newLayout = VkPipelineUtils::GetVkImgLayout(desc.newLayout);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = VkImage(desc.image->Image());
		barrier.subresourceRange.aspectMask = VkImageUtils::GetVkAspectMask(desc.aspect);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(m_buffer, VkPipelineUtils::GetVkPipelineStageFlags(desc.srcStage), VkPipelineUtils::GetVkPipelineStageFlags(desc.dstStage), 0,
			0, nullptr, 0, nullptr, 1, &barrier);
	}
}
