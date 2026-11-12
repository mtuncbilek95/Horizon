#include "MaterialStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Material/MaterialSerializer.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Buffer/BufferDesc.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Sampler/SamplerDesc.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Asset
{
	void MaterialStreamer::LoadAsync(Material* pAsset)
	{
		if (m_device == nullptr || !m_materialBuffer)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "No device! No Material Buffer!", pAsset->m_id.ToString());
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

	void MaterialStreamer::UnloadAsync(Material* pAsset)
	{
		ReleaseAsset(pAsset);
	}

	void MaterialStreamer::AcquireTextures(Material* pAsset)
	{
		auto* pAssets = GetEngine()->RequestService<Engine::AssetService>();
		if (!pAssets || pAsset->m_slotIndex == kInvalid32)
			return;

		MaterialParameters& parameters = pAsset->m_parameters;
		SlotPermits& slot = m_slotPermits[pAsset->m_slotIndex];

		MaterialData data = BuildData(parameters);
		data.baseColorIndex = ResolveTexture(pAssets, parameters.m_baseColorTexture, slot.permits[0]);
		data.normalIndex = ResolveTexture(pAssets, parameters.m_normalTexture, slot.permits[1]);
		data.emissiveIndex = ResolveTexture(pAssets, parameters.m_emissiveTexture, slot.permits[2]);
		data.occlusionIndex = ResolveTexture(pAssets, parameters.m_occlusionTexture, slot.permits[3]);
		data.roughnessIndex = ResolveTexture(pAssets, parameters.m_roughnessTexture, slot.permits[4]);
		data.metallicIndex = ResolveTexture(pAssets, parameters.m_metallicTexture, slot.permits[5]);

		WriteSlot(pAsset->m_slotIndex, data);
	}

	void MaterialStreamer::ReleaseTextures(Material* pAsset)
	{
		if (pAsset->m_slotIndex == kInvalid32)
			return;

		SlotPermits& slot = m_slotPermits[pAsset->m_slotIndex];

		for (u32 i = 0; i < kTextureSlotCount; ++i)
		{
			if (slot.permits[i].IsValid())
				slot.permits[i].pAsset->EndUse(slot.permits[i]);
		}
	}

	void MaterialStreamer::OnInitialize()
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

		RHI::BufferDesc bufferDesc = {};
		bufferDesc.size = sizeof(MaterialData) * kMaxMaterialCount;
		bufferDesc.usage = RHI::BufferUsage::Storage;
		bufferDesc.memory = RHI::MemoryType::Upload;
		m_materialBuffer = m_device->CreateRHIBuffer(bufferDesc);

		if (!m_materialBuffer || m_materialBuffer->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Material buffer could not be created, size: {}", bufferDesc.size);
			m_materialBuffer.Reset();
			return;
		}

		m_materialBuffer->SetDebugName("MaterialStreamer_MaterialBuffer");

		m_materialBufferView = m_resourceHeap->Allocate();
		m_resourceHeap->Write(m_materialBufferView, RHI::DescriptorType::Buffer, m_materialBuffer.Get(), 0, bufferDesc.size, sizeof(MaterialData));

		m_slotPermits = List<SlotPermits>(kMaxMaterialCount);

		MaterialParameters defaults;
		WriteSlot(0, BuildData(defaults));
		m_slotCount = 1;

		RHI::SamplerDesc samplerDesc = {};
		samplerDesc.maxAnisotropy = 16;
		m_defaultSampler = m_device->CreateRHISampler(samplerDesc);

		if (!m_defaultSampler)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Default material sampler could not be created");
			return;
		}

		m_samplerHeap = pGraphics->GetSamplerHeap();
		m_defaultSamplerView = m_samplerHeap->Allocate();
		m_samplerHeap->Write(m_defaultSamplerView, m_defaultSampler.Get());
	}

	void MaterialStreamer::OnFinalize()
	{
		if (m_defaultSamplerView != kInvalid32)
		{
			m_samplerHeap->Free(m_defaultSamplerView);
			m_defaultSamplerView = kInvalid32;
		}

		m_defaultSampler.Reset();
		m_samplerHeap = nullptr;

		if (m_materialBufferView != kInvalid32)
		{
			m_resourceHeap->Free(m_materialBufferView);
			m_materialBufferView = kInvalid32;
		}

		m_materialBuffer.Reset();
		m_freeSlots.Clear();
		m_slotPermits.Clear();
		m_slotCount = 0;

		m_resourceHeap = nullptr;
		m_device = nullptr;
		m_jobSystem = nullptr;
	}

	void MaterialStreamer::OnRegisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMaterialAsset = static_cast<Material*>(entry.pAssetObject);

		pMaterialAsset->m_id = entry.assetId;
		pMaterialAsset->m_cookPath = entry.cookPath;
	}

	void MaterialStreamer::OnUnregisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMaterialAsset = static_cast<Material*>(entry.pAssetObject);

		if (pMaterialAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMaterialAsset->m_ticket);

		ReleaseAsset(pMaterialAsset);
	}

	void MaterialStreamer::OnRefreshAsset(const Engine::AssetEntry& entry)
	{
		auto* pMaterialAsset = static_cast<Material*>(entry.pAssetObject);

		if (pMaterialAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMaterialAsset->m_ticket);

		ReleaseAsset(pMaterialAsset);
	}

	void MaterialStreamer::RunLoadAsset(Material* pAsset)
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

		if (header.propertySize != sizeof(MaterialProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "MaterialProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Header offsets point outside of the cook file", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		BinaryArchiveReader reader(view.GetData() + header.payloadOffset, header.payloadSize);
		const b8 wasRead = MaterialSerializer::Deserialize(pAsset->m_properties, pAsset->m_parameters, GetEngine()->GetReflectionSystem(), reader);

		PAL::File::CloseMap(view);

		if (!wasRead || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Material payload could not be deserialized", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		u32 slotIndex = kInvalid32;

		{
			ScopedLock lockSlots(m_lock);

			if (!m_freeSlots.IsEmpty())
			{
				slotIndex = m_freeSlots.Back();
				m_freeSlots.PopBack();
			}
			else if (m_slotCount < kMaxMaterialCount)
			{
				slotIndex = m_slotCount;
				++m_slotCount;
			}
		}

		if (slotIndex == kInvalid32)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Material buffer has no free slot", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		WriteSlot(slotIndex, BuildData(pAsset->m_parameters));
		pAsset->m_slotIndex = slotIndex;

		pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void MaterialStreamer::ReleaseAsset(Material* pAsset)
	{
		ReleaseTextures(pAsset);

		{
			ScopedLock lockSlots(m_lock);

			if (pAsset->m_slotIndex != kInvalid32)
				m_freeSlots.PushBack(pAsset->m_slotIndex);
		}

		pAsset->m_slotIndex = kInvalid32;
		pAsset->m_useCount.Store(0, PAL::MemoryOrder::Release);
		pAsset->m_residency.Store(Engine::AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	MaterialData MaterialStreamer::BuildData(const MaterialParameters& parameters) const
	{
		MaterialData data = {};
		data.baseColorFactor = parameters.m_baseColorFactor;
		data.emissiveFactor = parameters.m_emissiveFactor;
		data.metallic = parameters.m_metallic;
		data.roughness = parameters.m_roughness;
		data.normalScale = parameters.m_normalScale;
		data.alphaCutoff = parameters.m_alphaCutoff;
		data.baseColorIndex = kInvalid32;
		data.normalIndex = kInvalid32;
		data.emissiveIndex = kInvalid32;
		data.occlusionIndex = kInvalid32;
		data.roughnessIndex = kInvalid32;
		data.metallicIndex = kInvalid32;
		data.occlusionChannel = parameters.m_occlusionChannel;
		data.roughnessChannel = parameters.m_roughnessChannel;
		data.metallicChannel = parameters.m_metallicChannel;
		return data;
	}

	void MaterialStreamer::WriteSlot(u32 slotIndex, const MaterialData& data)
	{
		u8* pMapped = static_cast<u8*>(m_materialBuffer->GetMappedData());
		std::memcpy(pMapped + sizeof(MaterialData) * slotIndex, &data, sizeof(MaterialData));
	}

	u32 MaterialStreamer::ResolveTexture(Engine::AssetService* pAssets, Engine::AssetHandle<Texture>& handle, TextureUsePermit& permit)
	{
		if (handle.GetId() == Guid())
			return kInvalid32;

		Texture* pTexture = pAssets->Resolve(handle);
		if (!pTexture)
			return kInvalid32;

		if (pTexture->GetResidency() == Engine::AssetResidency::Unloaded)
		{
			pTexture->LoadAsync();
			return kInvalid32;
		}

		permit = pTexture->BeginUse();
		if (!permit.IsValid())
			return kInvalid32;

		return permit.viewIndex;
	}
}