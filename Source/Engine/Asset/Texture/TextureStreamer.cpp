#include "TextureStreamer.h"

#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Buffer/BufferDesc.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandListDesc.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Command/CommandPoolDesc.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Fence/FenceDesc.h>

namespace Horizon::Asset
{
	void TextureStreamer::LoadAsync(Texture* pAsset)
	{
		if (m_device == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "No device! No Vertex Buffer! No Index Buffer!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_ticket = m_jobSystem->SubmitJob(Engine::JobLane::Critical, Engine::Job([this, pAsset]()
			{
				RunLoadAsset(pAsset);
			}));

		if (pAsset->m_ticket == Engine::InvalidSubmitTicket)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Engine::Job could not be submitted", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void TextureStreamer::UnloadAsync(Texture* pAsset)
	{
		ReleaseAsset(pAsset);
	}

	void TextureStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();

		auto* pGraphics = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (!pGraphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Could not find Graphics Context!");
			return;
		}

		m_device = pGraphics->GetDevice();
		m_resourceHeap = pGraphics->GetResourceHeap();

		RHI::FenceDesc fenceDesc = {};
		m_uploadFence = m_device->CreateRHIFence(fenceDesc);
		m_uploadFence->SetDebugName("TextureStreamer_UploadFence");
	}

	void TextureStreamer::OnExecute(const Engine::EngineFrame& frame)
	{
		List<UniquePtr<TextureContext>> expired;

		{
			ScopedLock lockContext(m_lock);

			++m_tick;

			for (usize i = m_retiredContexts.GetCount(); i > 0; --i)
			{
				RetiredContext& retired = m_retiredContexts[i - 1];

				if (m_tick - retired.retiredTick < kRetireTicks)
					continue;

				expired.PushBack(std::move(retired.context));

				if (i - 1 != m_retiredContexts.GetCount() - 1)
					m_retiredContexts[i - 1] = std::move(m_retiredContexts.Back());

				m_retiredContexts.PopBack();
			}
		}

		for (usize i = 0; i < expired.GetCount(); ++i)
		{
			if (expired[i]->m_viewIndex != kInvalid32)
				m_resourceHeap->Free(expired[i]->m_viewIndex);
		}
	}


	void TextureStreamer::OnFinalize()
	{
		{
			ScopedLock lockContext(m_lock);

			for (usize i = 0; i < m_availableContexts.GetCount(); ++i)
			{
				if (m_availableContexts[i]->m_viewIndex != kInvalid32)
					m_resourceHeap->Free(m_availableContexts[i]->m_viewIndex);
			}

			for (usize i = 0; i < m_retiredContexts.GetCount(); ++i)
			{
				if (m_retiredContexts[i].context->m_viewIndex != kInvalid32)
					m_resourceHeap->Free(m_retiredContexts[i].context->m_viewIndex);
			}

			m_availableContexts.Clear();
			m_retiredContexts.Clear();
		}

		m_uploadFence.Reset();

		m_resourceHeap = nullptr;
		m_device = nullptr;
		m_jobSystem = nullptr;
	}

	void TextureStreamer::OnRegisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pTextureAsset = static_cast<Texture*>(entry.pAssetObject);

		pTextureAsset->m_id = entry.assetId;
		pTextureAsset->m_cookPath = entry.cookPath;
	}

	void TextureStreamer::OnUnregisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<Texture*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void TextureStreamer::OnRefreshAsset(const Engine::AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<Texture*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void TextureStreamer::RunLoadAsset(Texture* pAsset)
	{
		pAsset->m_residency.Store(Engine::AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		PAL::FileView view = PAL::File::OpenMap(fileHandl);
		PAL::File::ReleaseAccess(fileHandl);

		if (!view.IsValid())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Failed to map the cook file", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (view.GetSize() < sizeof(Engine::AssetHeader))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Cook file is smaller than its header", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		Engine::AssetHeader header;
		std::memcpy(&header, view.GetData(), sizeof(Engine::AssetHeader));

		if (header.magic != Engine::AssetHeader::Magic)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (header.version != Engine::AssetHeader::Version)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (header.propertySize != sizeof(TextureProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "TextureProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize tableEnd = header.tableOffset + header.tableSize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || tableEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Header offsets point outside of the cook file", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		TextureProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(TextureProperties));

		if (props.format == RHI::TextureFormat::Undefined || props.width == 0 || props.height == 0 || props.mipCount == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "TextureProperties has no valid pixel description", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (header.tableSize != props.mipCount * sizeof(TextureMip))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Mip table size does not match mipCount", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		RHI::TextureDesc textureDesc = {};
		textureDesc.type = RHI::TextureType::Texture2D;
		textureDesc.format = props.format;
		textureDesc.usage = RHI::TextureUsage::Sampled | RHI::TextureUsage::CopyDst;
		textureDesc.width = props.width;
		textureDesc.height = props.height;
		textureDesc.mipLevels = props.mipCount;

		UniquePtr<RHI::Texture> texture = m_device->CreateRHITexture(textureDesc);

		RHI::TextureViewDesc viewDesc = {};
		viewDesc.pTexture = texture.Get();
		viewDesc.type = RHI::TextureViewType::Texture2D;
		viewDesc.format = props.format;

		UniquePtr<RHI::TextureView> textureView = m_device->CreateRHITextureView(viewDesc);

		List<TextureMip> mips(props.mipCount);
		std::memcpy(mips.GetData(), view.GetData() + header.tableOffset, header.tableSize);

		for (u32 i = 0; i < props.mipCount; ++i)
		{
			if (mips[i].offset + mips[i].size > header.payloadSize)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Mip points outside of the payload", pAsset->m_id.ToString());
				pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}
		}

		if (!UploadPayload(view.GetData() + header.payloadOffset, header.payloadSize, mips, texture.Get()))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Payload could not be uploaded", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		PAL::File::CloseMap(view);

		UniquePtr<TextureContext> context = MakeUnique<TextureContext>();
		context->m_pointedAsset = pAsset;
		context->m_texture = std::move(texture);
		context->m_view = std::move(textureView);
		context->m_viewIndex = m_resourceHeap->Allocate();
		m_resourceHeap->Write(context->m_viewIndex, RHI::DescriptorType::Texture, context->m_view.Get());

		pAsset->m_context = context.Get();

		{
			ScopedLock lockContext(m_lock);
			pAsset->m_availableIndex = m_availableContexts.GetCount();
			m_availableContexts.PushBack(std::move(context));
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void TextureStreamer::ReleaseAsset(Texture* pAsset)
	{
		{
			ScopedLock lockContext(m_lock);

			if (pAsset->m_context != nullptr)
			{
				const usize index = pAsset->m_availableIndex;
				const usize last = m_availableContexts.GetCount() - 1;

				m_retiredContexts.EmplaceBack(std::move(m_availableContexts[index]), m_tick);

				if (index != last)
				{
					m_availableContexts[index] = std::move(m_availableContexts[last]);
					m_availableContexts[index]->m_pointedAsset->m_availableIndex = index;
				}

				m_availableContexts.PopBack();
			}
		}

		pAsset->m_context = nullptr;
		pAsset->m_availableIndex = 0;
		pAsset->m_useCount.Store(0, PAL::MemoryOrder::Release);
		pAsset->m_residency.Store(Engine::AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	b8 TextureStreamer::UploadPayload(const u8* pPayload, usize payloadSize, const List<TextureMip>& mips, RHI::Texture* pTexture)
	{
		RHI::BufferDesc stagingDesc = {};
		stagingDesc.size = payloadSize;
		stagingDesc.usage = RHI::BufferUsage::CopySrc;
		stagingDesc.memory = RHI::MemoryType::Upload;
		UniquePtr<RHI::Buffer> pStaging = m_device->CreateRHIBuffer(stagingDesc);

		if (!pStaging || pStaging->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Staging buffer could not be created, size: {}", stagingDesc.size);
			return false;
		}

		std::memcpy(pStaging->GetMappedData(), pPayload, payloadSize);

		RHI::CommandPoolDesc poolDesc = {};
		poolDesc.type = RHI::QueueType::Transfer;
		UniquePtr<RHI::CommandPool> pPool = m_device->CreateRHICommandPool(poolDesc);

		RHI::CommandListDesc listDesc = {};
		listDesc.pPool = pPool.Get();
		UniquePtr<RHI::CommandList> pCmd = m_device->CreateRHICommandList(listDesc);

		if (!pPool || !pCmd)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Transfer command list could not be created");
			return false;
		}

		RHI::TextureBarrier barrier = {};
		barrier.pTexture = pTexture;
		barrier.before = RHI::ResourceState::Undefined;
		barrier.after = RHI::ResourceState::Common;

		pCmd->Start();
		pCmd->Barrier({ barrier });

		for (u32 i = 0; i < mips.GetCount(); ++i)
			pCmd->CopyBufferToTexture(pStaging.Get(), mips[i].offset, pTexture, i);

		pCmd->End();

		u64 fenceValue = 0;

		{
			ScopedLock lockUpload(m_lock);
			m_device->Submit(RHI::QueueType::Transfer, { pCmd.Get() });
			fenceValue = m_device->Signal(RHI::QueueType::Transfer, m_uploadFence.Get());
		}

		m_uploadFence->WaitCPU(fenceValue);
		return true;
	}
}