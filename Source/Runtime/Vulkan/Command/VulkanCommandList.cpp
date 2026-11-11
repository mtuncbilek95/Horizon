#include "VulkanCommandList.h"
#include "vulkan/vulkan_core.h"

#include <Runtime/Vulkan/Buffer/VulkanBuffer.h>
#include <Runtime/Vulkan/Helpers/VulkanHelpers.h>
#include <Runtime/Vulkan/Command/VulkanCommandPool.h>
#include <Runtime/Vulkan/Descriptor/VulkanDescriptorHeap.h>
#include <Runtime/Vulkan/Descriptor/VulkanDescriptorTable.h>
#include <Runtime/Vulkan/Device/VulkanDevice.h>
#include <Runtime/Vulkan/Helpers/VulkanHelpers.h>
#include <Runtime/Vulkan/Pipeline/VulkanComputePipeline.h>
#include <Runtime/Vulkan/Pipeline/VulkanGraphicsPipeline.h>
#include <Runtime/Vulkan/Pipeline/VulkanPipelineLayout.h>
#include <Runtime/Vulkan/Texture/VulkanTexture.h>
#include <Runtime/Vulkan/Texture/VulkanTextureView.h>
#include <Runtime/RHI/Command/IndirectArguments.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Horizon::RHI
{
	static_assert(sizeof(DrawArguments) == sizeof(VkDrawIndirectCommand));
	static_assert(sizeof(DrawIndexedArguments) == sizeof(VkDrawIndexedIndirectCommand));
	static_assert(sizeof(DispatchArguments) == sizeof(VkDispatchIndirectCommand));

	VulkanCommandList::~VulkanCommandList()
	{
		if (m_cmdBuffer)
			vkFreeCommandBuffers(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), static_cast<VulkanCommandPool *>(m_pool)->Handle(), 1, &m_cmdBuffer);
	}

	void VulkanCommandList::SetDebugName(const c8* pName)
	{
		if (!m_cmdBuffer || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
            .objectHandle = (u64)m_cmdBuffer,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
	}

	void VulkanCommandList::Start()
	{
		const VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			// TODO:.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};

		vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);
		m_boundPipeline = nullptr;
	}

	void VulkanCommandList::End()
	{
		vkEndCommandBuffer(m_cmdBuffer);
	}

	void VulkanCommandList::BeginEvent(const c8* pName)
	{
		const VkDebugUtilsLabelEXT labelInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
			.pLabelName = pName,
			.color = { 1.f, 0.f, 0.f, 1.f }
		};
		
		vkCmdBeginDebugUtilsLabelEXT(m_cmdBuffer, &labelInfo);
	}

	void VulkanCommandList::EndEvent()
	{
		vkCmdEndDebugUtilsLabelEXT(m_cmdBuffer);
	}

	void VulkanCommandList::Barrier(ReadOnlyList<TextureBarrier> barriers)
	{
		if (barriers.IsEmpty())
			return;

		m_imageBarriers.Clear();

		for (const TextureBarrier& barrier : barriers)
		{
			VulkanTexture* pTexture = static_cast<VulkanTexture*>(barrier.pTexture);

			VkImageMemoryBarrier2 vkBarrier = {};
			vkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			vkBarrier.srcStageMask = Helpers::ToPipelineStageFlags(barrier.before);
			vkBarrier.dstStageMask = Helpers::ToPipelineStageFlags(barrier.after);
			vkBarrier.srcAccessMask = Helpers::ToAccessFlags(barrier.before);
			vkBarrier.dstAccessMask = Helpers::ToAccessFlags(barrier.after);
			vkBarrier.oldLayout = Helpers::ToImageLayout(barrier.before);
			vkBarrier.newLayout = Helpers::ToImageLayout(barrier.after);
			vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBarrier.image = pTexture->Handle();
			vkBarrier.subresourceRange.aspectMask = Helpers::ToAspectMask(pTexture->GetFormat());
			vkBarrier.subresourceRange.baseMipLevel = barrier.baseMip;
			vkBarrier.subresourceRange.levelCount = barrier.mipCount == kInvalid32 ? VK_REMAINING_MIP_LEVELS : barrier.mipCount;
			vkBarrier.subresourceRange.baseArrayLayer = barrier.baseLayer;
			vkBarrier.subresourceRange.layerCount = barrier.layerCount == kInvalid32 ? VK_REMAINING_ARRAY_LAYERS : barrier.layerCount;

			m_imageBarriers.PushBack(vkBarrier);

		}

		VkDependencyInfo dependencyInfo = {};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = static_cast<u32>(m_imageBarriers.GetCount());
		dependencyInfo.pImageMemoryBarriers = m_imageBarriers.GetData();

		vkCmdPipelineBarrier2(m_cmdBuffer, &dependencyInfo);
	}

	void VulkanCommandList::Barrier(ReadOnlyList<BufferBarrier> barriers)
	{
		if (barriers.IsEmpty())
			return;

		m_bufferBarriers.Clear();

		for (const BufferBarrier& barrier : barriers)
		{
			VulkanBuffer* pBuffer = static_cast<VulkanBuffer*>(barrier.pBuffer);

			VkBufferMemoryBarrier2 vkBarrier = {};
			vkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
			vkBarrier.srcStageMask = Helpers::ToPipelineStageFlags(barrier.before);
			vkBarrier.dstStageMask = Helpers::ToPipelineStageFlags(barrier.after);
			vkBarrier.srcAccessMask = Helpers::ToAccessFlags(barrier.before);
			vkBarrier.dstAccessMask = Helpers::ToAccessFlags(barrier.after);
			vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBarrier.buffer = pBuffer->Handle();
			vkBarrier.offset = 0;
			vkBarrier.size = VK_WHOLE_SIZE;

			m_bufferBarriers.PushBack(vkBarrier);
		}

		VkDependencyInfo dependencyInfo = {};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.bufferMemoryBarrierCount = static_cast<u32>(m_bufferBarriers.GetCount());
		dependencyInfo.pBufferMemoryBarriers = m_bufferBarriers.GetData();

		vkCmdPipelineBarrier2(m_cmdBuffer, &dependencyInfo);
	}

	void VulkanCommandList::BarrierUav()
	{
		VkMemoryBarrier2 memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		memoryBarrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;

		VkDependencyInfo dependencyInfo = {};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.memoryBarrierCount = 1;
		dependencyInfo.pMemoryBarriers = &memoryBarrier;

		vkCmdPipelineBarrier2(m_cmdBuffer, &dependencyInfo);
	}

	void VulkanCommandList::StartRendering(const RenderingDesc& desc)
	{
		VkRenderingAttachmentInfo colorAttachments[kMaxColorAttachments]{};
		VkRenderingAttachmentInfo depthAttachment{};

		for (u32 i = 0; i < desc.colorAttachmentCount; ++i)
		{
			const ColorAttachment& attachment = desc.colorAttachments[i];

			VulkanTextureView* pView = static_cast<VulkanTextureView*>(attachment.pView);

			colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachments[i].imageView = pView->GetHandle();
			colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachments[i].loadOp = Helpers::ToAttachmentLoadOp(attachment.loadOp);
			colorAttachments[i].storeOp = Helpers::ToAttachmentStoreOp(attachment.storeOp);
			colorAttachments[i].clearValue.color = VkClearColorValue{ attachment.clearColor[0], attachment.clearColor[1], attachment.clearColor[2], attachment.clearColor[3] };
		}

		if (desc.depthAttachment.pView)
		{
			const DepthAttachment& attachment = desc.depthAttachment;
			VulkanTextureView* pView = static_cast<VulkanTextureView*>(attachment.pView);

			depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = pView->GetHandle();
			depthAttachment.imageLayout = desc.depthAttachment.readOnly ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			depthAttachment.loadOp = Helpers::ToAttachmentLoadOp(attachment.loadOp);
			depthAttachment.storeOp = Helpers::ToAttachmentStoreOp(attachment.storeOp);
			depthAttachment.clearValue.depthStencil = VkClearDepthStencilValue{ attachment.clearDepth, attachment.clearStencil };
		}

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.layerCount = 1;
		renderingInfo.pColorAttachments = colorAttachments;
		renderingInfo.colorAttachmentCount = desc.colorAttachmentCount;
		renderingInfo.pStencilAttachment = desc.depthAttachment.pView ? (Helpers::IsStencilFormat(desc.depthAttachment.pView->GetFormat()) ? &depthAttachment : nullptr) : nullptr;
		renderingInfo.pDepthAttachment = desc.depthAttachment.pView ? &depthAttachment : nullptr;
		
		const TextureView *pView = desc.colorAttachments[0].pView ? desc.colorAttachments[0].pView : desc.depthAttachment.pView;
		renderingInfo.renderArea.extent = { 
			pView->GetTexture()->GetWidth() >> pView->GetBaseMip(), 
			pView->GetTexture()->GetHeight() >> pView->GetBaseMip()
		};
		
		vkCmdBeginRendering(m_cmdBuffer, &renderingInfo);
	}

	void VulkanCommandList::EndRendering()
	{
		vkCmdEndRendering(m_cmdBuffer);
	}

	void VulkanCommandList::BindDescriptorHeaps(DescriptorHeap* pResourceHeap, DescriptorHeap* pSamplerHeap)
	{
		const VulkanDescriptorHeap* pVkResourceHeap = static_cast<const VulkanDescriptorHeap*>(pResourceHeap);
		const VulkanDescriptorHeap* pVkSamplerHeap = static_cast<const VulkanDescriptorHeap*>(pSamplerHeap);

		if (pResourceHeap) {
			const u64 descriptorBytes = static_cast<u64>(pVkResourceHeap->GetCapacity()) * pVkResourceHeap->GetStride();

			VkBindHeapInfoEXT heapBindInfo{};
			heapBindInfo.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
			heapBindInfo.heapRange = {
				pVkResourceHeap->GpuAddress(),
				descriptorBytes + pVkResourceHeap->GetReservedForDriver()
			};
			heapBindInfo.reservedRangeOffset = descriptorBytes;
			heapBindInfo.reservedRangeSize = pVkResourceHeap->GetReservedForDriver();

			vkCmdBindResourceHeapEXT(m_cmdBuffer, &heapBindInfo);
		}

		if (pSamplerHeap) {
			const u64 descriptorBytes = static_cast<u64>(pVkSamplerHeap->GetCapacity()) * pVkSamplerHeap->GetStride();

			VkBindHeapInfoEXT heapBindInfo{};
			heapBindInfo.sType = VK_STRUCTURE_TYPE_BIND_HEAP_INFO_EXT;
			heapBindInfo.heapRange = {
				pVkSamplerHeap->GpuAddress(),
				descriptorBytes + pVkSamplerHeap->GetReservedForDriver()
			};
			heapBindInfo.reservedRangeOffset = descriptorBytes;
			heapBindInfo.reservedRangeSize = pVkSamplerHeap->GetReservedForDriver();

			vkCmdBindSamplerHeapEXT(m_cmdBuffer, &heapBindInfo);
		}
	}

	void VulkanCommandList::BindPipeline(Pipeline* pPipeline)
	{
		if (pPipeline->GetType() == PipelineType::Graphics)
		{
			vkCmdBindPipeline(m_cmdBuffer, Helpers::ToPipelineBindPoint(pPipeline->GetType()), static_cast<VulkanGraphicsPipeline*>(pPipeline)->Handle());
		}
		if (pPipeline->GetType() == PipelineType::Compute)
		{
			vkCmdBindPipeline(m_cmdBuffer, Helpers::ToPipelineBindPoint(pPipeline->GetType()), static_cast<VulkanComputePipeline*>(pPipeline)->Handle());
		}

		m_boundPipeline = pPipeline;
	}

	void VulkanCommandList::BindDescriptorTable(u32 tableIndex, DescriptorTable* pTable)
	{
		if (!m_boundPipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "BindDescriptorTable needs a bound pipeline, table: {}", tableIndex);
			return;
		}

		const VulkanPipelineLayout* pLayout = static_cast<const VulkanPipelineLayout*>(m_boundPipeline->GetLayout());
		VulkanDescriptorTable* pVkTable = static_cast<VulkanDescriptorTable*>(pTable);
		const u32 tableOffset = pVkTable->GetOffset();

		VkPushDataInfoEXT pushDataInfo{};
		pushDataInfo.sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT;
		pushDataInfo.data = {&tableOffset, sizeof(u32)};
		pushDataInfo.offset = 256 - ((tableIndex + 1) * sizeof(u32));

		vkCmdPushDataEXT(m_cmdBuffer, &pushDataInfo);
	}

	void VulkanCommandList::SetPushConstants(const void* pData, u32 size, u32 offset)
	{
		if (!m_boundPipeline || m_boundPipeline->GetLayout()->GetPushConstantSize() == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "SetPushConstants needs a bound pipeline with push constants");
			return;
		}

		VkPushDataInfoEXT pushDataInfo{};
		pushDataInfo.sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT;
		pushDataInfo.pNext = nullptr;
		pushDataInfo.data = {pData, size};
		pushDataInfo.offset = offset;

		vkCmdPushDataEXT(m_cmdBuffer, &pushDataInfo);
	}

	void VulkanCommandList::SetViewport(const Viewport& viewport)
	{
		VkViewport vkViewport = {};
		vkViewport.x = viewport.x;
		vkViewport.y = viewport.height + viewport.y;
		vkViewport.width = viewport.width;
		vkViewport.height = -viewport.height;
		vkViewport.minDepth = viewport.minDepth;
		vkViewport.maxDepth = viewport.maxDepth;

		vkCmdSetViewport(m_cmdBuffer, 0, 1, &vkViewport);
	}

	void VulkanCommandList::SetScissor(const Scissor& scissor)
	{
		VkRect2D vkScissor = {};
		vkScissor.offset.x = scissor.x;
		vkScissor.offset.y = scissor.y;
		vkScissor.extent.width = scissor.width;
		vkScissor.extent.height = scissor.height;

		vkCmdSetScissor(m_cmdBuffer, 0, 1, &vkScissor);
	}

	void VulkanCommandList::BindVertexBuffer(u32 binding, Buffer* pBuffer, u64 offset)
	{
		if (!m_boundPipeline || m_boundPipeline->GetType() != PipelineType::Graphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "BindVertexBuffer needs a bound graphics pipeline, binding: {}", binding);
			return;
		}

		u32 stride = kInvalid32;
		for (const VertexBinding& vertexBinding : static_cast<GraphicsPipeline*>(m_boundPipeline)->GetVertexBindings())
		{
			if (vertexBinding.binding == binding)
				stride = vertexBinding.stride;
		}

		if (stride == kInvalid32)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Bound pipeline has no vertex binding {}", binding);
			return;
		}

		VkBuffer vkBuffer = static_cast<VulkanBuffer*>(pBuffer)->Handle();
		vkCmdBindVertexBuffers(m_cmdBuffer, binding, 1, &vkBuffer, &offset);
	}

	void VulkanCommandList::BindIndexBuffer(Buffer* pBuffer, IndexType type, u64 offset)
	{
		vkCmdBindIndexBuffer(m_cmdBuffer, static_cast<VulkanBuffer*>(pBuffer)->Handle(), offset, Helpers::ToIndexType(type));
	}

	void VulkanCommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		vkCmdDraw(m_cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void VulkanCommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
	{
		vkCmdDrawIndexed(m_cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandList::DrawIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		vkCmdDrawIndirect(m_cmdBuffer, static_cast<VulkanBuffer*>(pArgs)->Handle(), argsOffset, maxDrawCount, sizeof(DrawArguments));
	}

	void VulkanCommandList::DrawIndirectIndexed(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		vkCmdDrawIndexedIndirect(m_cmdBuffer, static_cast<VulkanBuffer*>(pArgs)->Handle(), argsOffset, maxDrawCount, sizeof(DrawArguments));
	}

	void VulkanCommandList::DispatchCompute(u32 groupX, u32 groupY, u32 groupZ)
	{
		vkCmdDispatch(m_cmdBuffer, groupX, groupY, groupZ);
	}

	void VulkanCommandList::DispatchComputeIndirect(Buffer* pArgs, u64 argsOffset)
	{
		vkCmdDispatchIndirect(m_cmdBuffer, static_cast<VulkanBuffer*>(pArgs)->Handle(), argsOffset);
	}

	void VulkanCommandList::DispatchMesh(u32 groupX, u32 groupY, u32 groupZ)
	{
		vkCmdDrawMeshTasksEXT(m_cmdBuffer, groupX, groupY, groupZ);
	}

	void VulkanCommandList::DispatchMeshIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		vkCmdDrawMeshTasksIndirectEXT(m_cmdBuffer, static_cast<VulkanBuffer*>(pArgs)->Handle(), argsOffset, maxDrawCount, sizeof(DispatchArguments));
	}

	void VulkanCommandList::CopyBuffer(Buffer* pSrc, u64 srcOffset, Buffer* pDst, u64 dstOffset, u64 size)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;

		vkCmdCopyBuffer(m_cmdBuffer, static_cast<VulkanBuffer*>(pSrc)->Handle(), static_cast<VulkanBuffer*>(pDst)->Handle(), 1, &copyRegion);
	}

	void VulkanCommandList::CopyBufferToTexture(Buffer* pSrc, u64 srcOffset, Texture* pDst, u32 mip, u32 layer)
	{
		VulkanTexture* pTexture = static_cast<VulkanTexture*>(pDst);
		VkBufferImageCopy region = {};
		region.bufferOffset = srcOffset;
		region.imageSubresource.aspectMask = Helpers::ToAspectMask(pTexture->GetFormat());
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = layer;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = pTexture->GetWidth();
		region.imageExtent.height = pTexture->GetHeight();
		region.imageExtent.depth = pTexture->GetDepth();
		region.imageOffset = { 0, 0, 0 };

		vkCmdCopyBufferToImage(m_cmdBuffer, static_cast<VulkanBuffer*>(pSrc)->Handle(), pTexture->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void VulkanCommandList::CopyTextureToBuffer(Texture* pSrc, u32 mip, u32 layer, Buffer* pDst, u64 dstOffset)
	{
		VulkanTexture* pTexture = static_cast<VulkanTexture*>(pSrc);
		VkBufferImageCopy region = {};
		region.bufferOffset = dstOffset;
		region.imageSubresource.aspectMask = Helpers::ToAspectMask(pTexture->GetFormat());
		region.imageSubresource.mipLevel = mip;
		region.imageSubresource.baseArrayLayer = layer;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = pTexture->GetWidth();
		region.imageExtent.height = pTexture->GetHeight();
		region.imageExtent.depth = pTexture->GetDepth();
		region.imageOffset = { 0, 0, 0 };

		vkCmdCopyImageToBuffer(m_cmdBuffer, pTexture->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, static_cast<VulkanBuffer*>(pDst)->Handle(), 1, &region);
	}

	void VulkanCommandList::ResolveTexture(Texture* pSrc, Texture* pDst, u32 mip, u32 layer) 
	{
		VkImageResolve resolveRegion{};
		resolveRegion.srcSubresource.aspectMask = Helpers::ToAspectMask(static_cast<VulkanTexture*>(pSrc)->GetFormat());
		resolveRegion.srcSubresource.mipLevel = mip;
		resolveRegion.srcSubresource.baseArrayLayer = layer;
		resolveRegion.srcSubresource.layerCount = 1;
		resolveRegion.dstSubresource = resolveRegion.srcSubresource;
		resolveRegion.dstOffset = { 0, 0, 0 };
		resolveRegion.extent = { static_cast<u32>(static_cast<VulkanTexture*>(pDst)->GetWidth()), static_cast<u32>(static_cast<VulkanTexture*>(pDst)->GetHeight()), static_cast<u32>(static_cast<VulkanTexture*>(pDst)->GetDepth()) };

		vkCmdResolveImage(m_cmdBuffer, static_cast<VulkanTexture*>(pSrc)->Handle(), VK_IMAGE_LAYOUT_GENERAL, static_cast<VulkanTexture*>(pDst)->Handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &resolveRegion);
	}

	void VulkanCommandList::RenderImGui(ImDrawData* pDrawData)
	{
		ImGui_ImplVulkan_RenderDrawData(pDrawData, m_cmdBuffer);

		m_boundPipeline = nullptr;
	}
}
