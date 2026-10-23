#include "MeshAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>

namespace Horizon::Engine
{
	void MeshAssetStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
		GraphicsContext* pContext = GetEngine()->RequestContext<GraphicsContext>();

		if (pContext == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, meshes cannot be loaded");
			return;
		}

		m_device = pContext->GetDevice();
		if (!m_device)
		{
			Terminal::Fatal(StringOps::GetName(this), "No graphics device, no mesh streamer");
			return;
		}

		RHI::GfxBufferArenaDesc vertArenaDesc = {};
		vertArenaDesc.capacity = MibToByte(512ull);
		vertArenaDesc.stride = sizeof(MeshVertex);
		vertArenaDesc.memory = RHI::GfxMemoryType::GpuUpload;
		vertArenaDesc.usage = RHI::GfxBufferUsage::Storage;
		m_vertexArena = m_device->CreateBufferArena(vertArenaDesc);
		m_vertexArena->SetDebugName("MeshStreamer_VertexArena");
		Terminal::Assert(m_vertexArena, StringOps::GetName(this), "Big fucked at some places");

		m_vertexMap = (u8*)m_vertexArena->GetBuffer()->Map();

		RHI::GfxBufferArenaDesc indexArenaDesc = {};
		indexArenaDesc.capacity = MibToByte(256ull);
		indexArenaDesc.stride = sizeof(u32);
		indexArenaDesc.memory = RHI::GfxMemoryType::GpuUpload;
		indexArenaDesc.usage = RHI::GfxBufferUsage::Index;
		m_indexArena = m_device->CreateBufferArena(indexArenaDesc);
		m_indexArena->SetDebugName("MeshStreamer_IndexArena");
		Terminal::Assert(m_indexArena, StringOps::GetName(this), "Big fucked at some places");

		m_indexMap = (u8*)m_indexArena->GetBuffer()->Map();
	}

	void MeshAssetStreamer::OnFinalize()
	{
		m_device = nullptr;

		Memory::Allocator::Delete(m_indexArena);
		Memory::Allocator::Delete(m_vertexArena);
	}

	void MeshAssetStreamer::OnRegisteredAsset(const AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(entry.pAssetObject);

		pMeshAsset->m_id = entry.assetId;
		pMeshAsset->m_cookPath = entry.cookPath;
	}

	void MeshAssetStreamer::OnUnregisteredAsset(const AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void MeshAssetStreamer::OnRefreshAsset(const AssetEntry& entry)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(entry.pAssetObject);

		if (pMeshAsset->m_ticket != InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pMeshAsset->m_ticket);

		ReleaseAsset(pMeshAsset);
	}

	void MeshAssetStreamer::LoadAsync(MeshAsset* pAsset)
	{
		if (m_device == nullptr || m_vertexArena == nullptr || m_indexArena == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "No device! No Vertex Arena! No Index Arena!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Critical, Job([this, pAsset]()
			{
				RunLoadAsset(pAsset);
			}));

		if (pAsset->m_ticket == InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Job could not be submitted", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void MeshAssetStreamer::UnloadAsync(MeshAsset* pAsset)
	{
		ReleaseAsset(pAsset);
	}

	void MeshAssetStreamer::ReleaseAsset(MeshAsset* pAsset)
	{
		{
			ScopedLock lockArena(m_arenaLock);

			if (pAsset->m_vertexRange.IsValid())
				m_vertexArena->Free(pAsset->m_vertexRange);

			if (pAsset->m_indexRange.IsValid())
				m_indexArena->Free(pAsset->m_indexRange);
		}

		pAsset->m_vertexRange = {};
		pAsset->m_indexRange = {};
		pAsset->m_submeshes.Clear();
		pAsset->m_properties = {};
		pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	void MeshAssetStreamer::RunLoadAsset(MeshAsset* pAsset)
	{
		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		PAL::FileView view = PAL::File::OpenMap(fileHandl);
		PAL::File::ReleaseAccess(fileHandl);

		if (!view.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to map the cook file", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (view.GetSize() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Cook file is smaller than its header", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		AssetHeader header;
		std::memcpy(&header, view.GetData(), sizeof(AssetHeader));

		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (header.propertySize != sizeof(MeshAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "MeshAssetProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize tableEnd = header.tableOffset + header.tableSize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || tableEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Header offsets point outside of the cook file", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		MeshAssetProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(MeshAssetProperties));

		if (props.vertexStride != sizeof(MeshVertex) || props.indexStride != sizeof(u32))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "VertexStride or IndexStride is not properly sized", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		const usize subMeshTable = props.subMeshCount * sizeof(MeshAssetSubmesh);
		const usize vertBlock = props.vertexCount * props.vertexStride;
		const usize indexBlock = props.indexCount * props.indexStride;

		if (subMeshTable != header.tableSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Table size does not match MeshAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		if (vertBlock + indexBlock != header.payloadSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Payload size does not match MeshAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		List<MeshAssetSubmesh> subMeshes(props.subMeshCount);
		std::memcpy(subMeshes.GetData(), view.GetData() + header.tableOffset, subMeshTable);

		for (usize i = 0; i < subMeshes.GetCount(); ++i)
		{
			const MeshAssetSubmesh& subMesh = subMeshes[i];

			const b8 hasValidIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 hasValidVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!hasValidIndices || !hasValidVertices)
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "a sub-mesh points outside of its buffers", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}
		}

		const u8* pVertexData = view.GetData() + header.payloadOffset;
		const u8* pIndexData = pVertexData + vertBlock;

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_arenaLock);

			RHI::GfxBufferRange vRange = m_vertexArena->Allocate(vertBlock, sizeof(MeshVertex));
			if (!vRange.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "VertexBufferArena is out of space", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}

			RHI::GfxBufferRange iRange = m_indexArena->Allocate(indexBlock, sizeof(u32));
			if (!iRange.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "IndexBufferArena is out of space", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				m_vertexArena->Free(vRange);
				PAL::File::CloseMap(view);
				return;
			}

			pAsset->m_vertexRange = vRange;
			pAsset->m_indexRange = iRange;
		}

		std::memcpy(m_vertexMap + pAsset->m_vertexRange.offset, pVertexData, vertBlock);
		std::memcpy(m_indexMap + pAsset->m_indexRange.offset, pIndexData, indexBlock);

		PAL::File::CloseMap(view);

		pAsset->m_submeshes = subMeshes;
		pAsset->m_properties = props;
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}
}