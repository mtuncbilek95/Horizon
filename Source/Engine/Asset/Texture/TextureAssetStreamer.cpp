#include "TextureAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Buffer/GfxBufferDesc.h>
#include <Runtime/RHI/Command/GfxTextureBarrier.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeapDesc.h>
#include <Runtime/RHI/Texture/GfxTextureDesc.h>
#include <Runtime/RHI/Texture/GfxTextureFootprint.h>

namespace Horizon::Engine
{
	void TextureAssetStreamer::LoadAsync(TextureAsset* pAsset)
	{
		if (m_device == nullptr || m_shaderHeap == nullptr || m_fence == nullptr || m_commandList == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "No device! No shader view heap! No upload fence!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Critical, Job([this, pAsset]()
			{
				RunLoadTexture(pAsset);
			}));

		if (pAsset->m_ticket == InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Job could not be submitted", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void TextureAssetStreamer::UnloadAsync(TextureAsset* pAsset)
	{
	}

	void TextureAssetStreamer::LoadSync(TextureAsset* pAsset)
	{
		if (m_device == nullptr || m_shaderHeap == nullptr || m_fence == nullptr || m_commandList == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "No device! No shader view heap! No upload fence!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		RunLoadTexture(pAsset);
	}

	void TextureAssetStreamer::UnloadSync(TextureAsset* pAsset)
	{
		m_device->WaitIdle();
		ReleaseAsset(pAsset);
	}

	void TextureAssetStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
		GraphicsContext* pContext = GetEngine()->RequestContext<GraphicsContext>();

		if (pContext == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, textures cannot be loaded");
			return;
		}

		m_device = pContext->GetDevice();
		m_queue = pContext->GetGraphicsQueue();

		if (!m_device || !m_queue)
		{
			Terminal::Fatal(StringOps::GetName(this), "No graphics device or queue, no texture streamer");
			m_device = nullptr;
			return;
		}

		RHI::GfxDescriptorHeapDesc heapDesc = {};
		heapDesc.type = RHI::GfxDescriptorHeapType::Resource;
		heapDesc.capacity = ShaderViewCapacity;
		m_shaderHeap = m_device->CreateDescriptorHeap(heapDesc);
		Terminal::Assert(m_shaderHeap, StringOps::GetName(this), "Texture shader view heap could not be created");

		m_fence = m_device->CreateFence();
		Terminal::Assert(m_fence, StringOps::GetName(this), "Texture upload fence could not be created");

		m_commandList = m_device->CreateCommandList(RHI::GfxQueueType::Graphics);
		Terminal::Assert(m_commandList, StringOps::GetName(this), "Texture upload command list could not be created");
	}

	void TextureAssetStreamer::OnFinalize()
	{
		if (m_device == nullptr)
			return;

		m_device->WaitIdle();

		Memory::Allocator::Delete(m_commandList);
		Memory::Allocator::Delete(m_fence);
		Memory::Allocator::Delete(m_shaderHeap);

		m_commandList = nullptr;
		m_fence = nullptr;
		m_shaderHeap = nullptr;
		m_device = nullptr;
	}

	void TextureAssetStreamer::OnRegisteredAsset(const AssetEntry& entry)
	{
		auto* pTextureAsset = static_cast<TextureAsset*>(entry.pAssetObject);

		pTextureAsset->m_id = entry.assetId;
		pTextureAsset->m_cookPath = entry.cookPath;
	}

	void TextureAssetStreamer::OnUnregisteredAsset(const AssetEntry& entry)
	{
		auto* pTextureAsset = static_cast<TextureAsset*>(entry.pAssetObject);

		if (pTextureAsset->m_texture == nullptr)
		{
			pTextureAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
			return;
		}

		m_device->WaitIdle();
		ReleaseAsset(pTextureAsset);
	}

	void TextureAssetStreamer::OnRefreshAsset(const AssetEntry& entry)
	{
		auto* pTextureAsset = static_cast<TextureAsset*>(entry.pAssetObject);

		if (pTextureAsset->m_texture == nullptr)
		{
			pTextureAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
			return;
		}

		m_device->WaitIdle();
		ReleaseAsset(pTextureAsset);
	}

	void TextureAssetStreamer::RunLoadTexture(TextureAsset* pAsset)
	{
		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		List<u8> headerBytes;
		if (!PAL::File::ReadMemory(fileHandl, headerBytes, 0, sizeof(AssetHeader)) || headerBytes.GetCount() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the AssetHeader", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		AssetHeader header;
		std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));

		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.propertySize != sizeof(TextureAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "TextureAssetProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> propertyBytes;
		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for TextureAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		TextureAssetProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(TextureAssetProperties));

		if (props.format == RHI::GfxTextureFormat::Undefined || props.textureSize.X() == 0 || props.textureSize.Y() == 0 || props.mipCount == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "TextureAssetProperties carry an undefined format, an empty size or no mips", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (props.mipCount * sizeof(TextureAssetMip) != header.tableSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Table size does not match the mip count", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (props.textureByteSize != header.payloadSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Payload size does not match TextureAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> tableBytes;
		if (!PAL::File::ReadMemory(fileHandl, tableBytes, header.tableOffset, header.tableOffset + sizeof(TextureAssetMip)))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the mip table", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		TextureAssetMip topMip;
		std::memcpy(&topMip, tableBytes.GetData(), sizeof(TextureAssetMip));

		const b8 hasValidSize = topMip.width == props.textureSize.X() && topMip.height == props.textureSize.Y();
		const b8 hasValidRange = topMip.byteSize != 0 && topMip.offset + topMip.byteSize <= header.payloadSize;

		if (!hasValidSize || !hasValidRange)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Top mip does not match TextureAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> mipBytes;
		const usize mipStart = header.payloadOffset + topMip.offset;
		if (!PAL::File::ReadMemory(fileHandl, mipBytes, mipStart, mipStart + topMip.byteSize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for the top mip", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		PAL::File::ReleaseAccess(fileHandl);

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		RHI::GfxTextureDesc textureDesc = {};
		textureDesc.type = RHI::GfxTextureType::Tex2D;
		textureDesc.format = props.format;
		textureDesc.usage = RHI::GfxTextureUsage::Sampled | RHI::GfxTextureUsage::TransferDst;
		textureDesc.width = props.textureSize.X();
		textureDesc.height = props.textureSize.Y();
		textureDesc.mipLevels = 1;

		const RHI::GfxTextureFootprint footprint = m_device->GetTextureFootprint(textureDesc);
		const b8 hasValidFootprint = footprint.IsValid() && topMip.byteSize % footprint.rowCount == 0;

		if (!hasValidFootprint || topMip.byteSize / footprint.rowCount > footprint.rowPitch)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Top mip rows do not match the GPU footprint", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		RHI::GfxTexture* pTexture = m_device->CreateTexture(textureDesc);
		if (!pTexture)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Texture could not be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}
		
		pTexture->SetDebugName(pAsset->m_id.ToString().c_str());

		RHI::GfxBufferDesc stagingDesc = {};
		stagingDesc.usage = RHI::GfxBufferUsage::TransferSrc;
		stagingDesc.memory = RHI::GfxMemoryType::Upload;
		stagingDesc.size = footprint.totalBytes;

		RHI::GfxBuffer* pStaging = m_device->CreateBuffer(stagingDesc);
		if (!pStaging)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Staging buffer could not be created", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			Memory::Allocator::Delete(pTexture);
			return;
		}

		u8* pMapped = (u8*)pStaging->Map();
		if (!pMapped)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Staging buffer could not be mapped", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			Memory::Allocator::Delete(pStaging);
			Memory::Allocator::Delete(pTexture);
			return;
		}

		const usize rowSize = topMip.byteSize / footprint.rowCount;
		for (u32 row = 0; row < footprint.rowCount; ++row)
			std::memcpy(pMapped + row * footprint.rowPitch, mipBytes.GetData() + row * rowSize, rowSize);

		pStaging->Unmap();

		ScopedLock lockTextures(m_textureLocker);
		{
			m_commandList->Begin();

			RHI::GfxTextureBarrier toCopy = {};
			toCopy.pTexture = pTexture;
			toCopy.before = RHI::GfxResourceState::Common;
			toCopy.after = RHI::GfxResourceState::CopyDst;
			m_commandList->Barrier(&toCopy, 1);

			m_commandList->CopyBufferToTexture(pStaging, 0, pTexture, 0);

			RHI::GfxTextureBarrier toShader = {};
			toShader.pTexture = pTexture;
			toShader.before = RHI::GfxResourceState::CopyDst;
			toShader.after = RHI::GfxResourceState::ShaderResource;
			m_commandList->Barrier(&toShader, 1);

			m_commandList->End();

			m_queue->Submit(&m_commandList, 1);
			m_fence->WaitCPU(m_queue->Signal(m_fence));

			Memory::Allocator::Delete(pStaging);

			const u32 shaderIndex = m_shaderHeap->CreateShaderView(pTexture);
			if (shaderIndex == kInvalid32)
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Shader view heap is out of space", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				Memory::Allocator::Delete(pTexture);
				return;
			}

			pAsset->m_properties = props;
			pAsset->m_texture = pTexture;
			pAsset->m_shaderIndex = shaderIndex;
		}
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void TextureAssetStreamer::ReleaseAsset(TextureAsset* pAsset)
	{
		ScopedLock lockTextures(m_textureLocker);

		if (pAsset->m_shaderIndex != kInvalid32)
			m_shaderHeap->Free(pAsset->m_shaderIndex);

		if (pAsset->m_texture)
			Memory::Allocator::Delete(pAsset->m_texture);

		pAsset->m_texture = nullptr;
		pAsset->m_shaderIndex = kInvalid32;
		pAsset->m_properties = {};
		pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}
}