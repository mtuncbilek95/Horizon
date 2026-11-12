#include "MeshStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Buffer/BufferDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandListDesc.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Command/CommandPoolDesc.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Fence/FenceDesc.h>

#include <cstring>

namespace Horizon::Asset
{
	void MeshStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();

		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Engine::GraphicsContext is unavailable, meshes cannot be loaded");
			return;
		}

		m_device = pContext->GetDevice();
		if (!m_device)
		{
			Terminal::Fatal(StringHelper::GetTypeName(this), "No graphics device, no mesh streamer");
			return;
		}

		m_resourceHeap = pContext->GetResourceHeap();

		RHI::BufferDesc vertexDesc = {};
		vertexDesc.size = MibToByte(512ull);
		vertexDesc.usage = RHI::BufferUsage::Storage | RHI::BufferUsage::CopyDst;
		vertexDesc.memory = RHI::MemoryType::GpuOnly;
		m_vertexBuffer = m_device->CreateRHIBuffer(vertexDesc);
		Terminal::Assert(m_vertexBuffer != nullptr, StringHelper::GetTypeName(this), "Big fucked at some places");
		m_vertexBuffer->SetDebugName("MeshStreamer_VertexBuffer");

		m_vertexBufferView = m_resourceHeap->Allocate();
		m_resourceHeap->Write(m_vertexBufferView, RHI::DescriptorType::Buffer, m_vertexBuffer.Get(), 0, vertexDesc.size, sizeof(MeshVertex));
		m_vertexRanges.Initialize(vertexDesc.size);

		RHI::BufferDesc indexDesc = {};
		indexDesc.size = MibToByte(256ull);
		indexDesc.usage = RHI::BufferUsage::Index | RHI::BufferUsage::CopyDst;
		indexDesc.memory = RHI::MemoryType::GpuOnly;
		m_indexBuffer = m_device->CreateRHIBuffer(indexDesc);
		Terminal::Assert(m_indexBuffer != nullptr, StringHelper::GetTypeName(this), "Big fucked at some places");
		m_indexBuffer->SetDebugName("MeshStreamer_IndexBuffer");

		m_indexRanges.Initialize(indexDesc.size);

		RHI::FenceDesc fenceDesc = {};
		m_uploadFence = m_device->CreateRHIFence(fenceDesc);
		m_uploadFence->SetDebugName("MeshStreamer_UploadFence");
	}

	void MeshStreamer::OnFinalize()
	{
		if (m_vertexBufferView != kInvalid32)
		{
			m_resourceHeap->Free(m_vertexBufferView);
			m_vertexBufferView = kInvalid32;
		}

		m_uploadFence.Reset();
		m_indexBuffer.Reset();
		m_vertexBuffer.Reset();

		m_resourceHeap = nullptr;
		m_device = nullptr;
	}

	void MeshStreamer::OnRegisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<Mesh*>(entry.pAssetObject);

		pMeshAsset->m_id = entry.assetId;
		pMeshAsset->m_cookPath = entry.cookPath;
	}

	void MeshStreamer::OnUnregisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<Mesh*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void MeshStreamer::OnRefreshAsset(const Engine::AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<Mesh*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void MeshStreamer::LoadAsync(Mesh* pAsset)
	{
		if (m_device == nullptr || !m_vertexBuffer || !m_indexBuffer)
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

	void MeshStreamer::UnloadAsync(Mesh* pAsset)
	{
		ReleaseAsset(pAsset);
	}

	void MeshStreamer::ReleaseAsset(Mesh* pAsset)
	{
		{
			ScopedLock lockArena(m_arenaLock);

			if (pAsset->m_vertexRange.IsValid())
				m_vertexRanges.Free(pAsset->m_vertexRange);

			if (pAsset->m_indexRange.IsValid())
				m_indexRanges.Free(pAsset->m_indexRange);
		}

		pAsset->m_vertexRange = {};
		pAsset->m_indexRange = {};
		pAsset->m_submeshes.Clear();
		pAsset->m_properties = {};
		pAsset->m_residency.Store(Engine::AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	void MeshStreamer::RunLoadAsset(Mesh* pAsset)
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

		if (header.propertySize != sizeof(MeshProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "MeshProperties are not matching!", pAsset->m_id.ToString());
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

		MeshProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(MeshProperties));

		if (props.vertexStride != sizeof(MeshVertex) || props.indexStride != sizeof(u32))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "VertexStride or IndexStride is not properly sized", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		const usize subMeshTable = props.subMeshCount * sizeof(MeshSubmesh);
		const usize vertBlock = props.vertexCount * props.vertexStride;
		const usize indexBlock = props.indexCount * props.indexStride;

		if (subMeshTable != header.tableSize)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Table size does not match MeshProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (vertBlock + indexBlock != header.payloadSize)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Payload size does not match MeshProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		List<MeshSubmesh> subMeshes(props.subMeshCount);
		std::memcpy(subMeshes.GetData(), view.GetData() + header.tableOffset, subMeshTable);

		for (usize i = 0; i < subMeshes.GetCount(); ++i)
		{
			const MeshSubmesh& subMesh = subMeshes[i];

			const b8 hasValidIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 hasValidVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!hasValidIndices || !hasValidVertices)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "a sub-mesh points outside of its buffers", pAsset->m_id.ToString());
				pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}
		}

		const u8* pVertexData = view.GetData() + header.payloadOffset;
		const u8* pIndexData = pVertexData + vertBlock;

		pAsset->m_residency.Store(Engine::AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		Range vRange;
		Range iRange;

		{
			ScopedLock lockArena(m_arenaLock);

			vRange = m_vertexRanges.Allocate(vertBlock, sizeof(MeshVertex));
			if (!vRange.IsValid())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Vertex buffer is out of space", pAsset->m_id.ToString());
				pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}

			iRange = m_indexRanges.Allocate(indexBlock, sizeof(u32));
			if (!iRange.IsValid())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Index buffer is out of space", pAsset->m_id.ToString());
				pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
				m_vertexRanges.Free(vRange);
				PAL::File::CloseMap(view);
				return;
			}
		}

		if (!UploadPayload(pVertexData, vRange, pIndexData, iRange))
		{
			{
				ScopedLock lockArena(m_arenaLock);
				m_vertexRanges.Free(vRange);
				m_indexRanges.Free(iRange);
			}

			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		PAL::File::CloseMap(view);

		pAsset->m_vertexRange = vRange;
		pAsset->m_indexRange = iRange;
		pAsset->m_submeshes = subMeshes;
		pAsset->m_properties = props;
		pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	b8 MeshStreamer::UploadPayload(const u8* pVertexData, const Range& vertexRange, const u8* pIndexData, const Range& indexRange)
	{
		RHI::BufferDesc stagingDesc = {};
		stagingDesc.size = vertexRange.size + indexRange.size;
		stagingDesc.usage = RHI::BufferUsage::CopySrc;
		stagingDesc.memory = RHI::MemoryType::Upload;
		UniquePtr<RHI::Buffer> pStaging = m_device->CreateRHIBuffer(stagingDesc);

		if (!pStaging || pStaging->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Staging buffer could not be created, size: {}", stagingDesc.size);
			return false;
		}

		std::memcpy(pStaging->GetMappedData(), pVertexData, vertexRange.size);
		std::memcpy(pStaging->GetMappedData() + vertexRange.size, pIndexData, indexRange.size);

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

		pCmd->Start();
		pCmd->CopyBuffer(pStaging.Get(), 0, m_vertexBuffer.Get(), vertexRange.offset, vertexRange.size);
		pCmd->CopyBuffer(pStaging.Get(), vertexRange.size, m_indexBuffer.Get(), indexRange.offset, indexRange.size);
		pCmd->End();

		u64 fenceValue = 0;

		{
			ScopedLock lockUpload(m_uploadLock);
			m_device->Submit(RHI::QueueType::Transfer, { pCmd.Get() });
			fenceValue = m_device->Signal(RHI::QueueType::Transfer, m_uploadFence.Get());
		}

		m_uploadFence->WaitCPU(fenceValue);
		return true;
	}
}