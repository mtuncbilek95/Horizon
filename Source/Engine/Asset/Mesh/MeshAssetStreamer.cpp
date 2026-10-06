#include "MeshAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

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

	void MeshAssetStreamer::OnSync(const EngineFrame& frameContext)
	{
	}

	void MeshAssetStreamer::OnFinalize()
	{
		m_device = nullptr;

		m_resident.Clear();

		Memory::Allocator::Delete(m_indexArena);
		Memory::Allocator::Delete(m_vertexArena);
	}

	void MeshAssetStreamer::LoadAsync(AssetObject* pAsset)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(pAsset);

		if (m_device == nullptr || m_vertexArena == nullptr || m_indexArena == nullptr)
		{
			FailedAssetLog(pMeshAsset, "No device! No Vertex Arena! No Index Arena!");
			return;
		}

		pMeshAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Critical, Job([this, pMeshAsset]()
			{
				RunLoadAsset(pMeshAsset);
			}));

		if (pMeshAsset->m_ticket == InvalidSubmitTicket)
			FailedAssetLog(pMeshAsset, "Job could not be submitted");
	}

	void MeshAssetStreamer::UnloadAsync(AssetObject* pAsset)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(pAsset);

		ScopedLock lockArena(m_arenaLock);

		if (!Evict(pMeshAsset))
			Terminal::Warn(StringOps::GetName(this), "{} is in use or not resident, unload skipped", pMeshAsset->m_ownerEntry.assetId.ToString());
	}

	PermittedMeshData MeshAssetStreamer::BeginUse(MeshAsset* pAsset)
	{
		PermittedMeshData permit = {};

		pAsset->m_useCount.FetchAdd(1, PAL::MemoryOrder::SeqCst);

		if (pAsset->m_residency.Load(PAL::MemoryOrder::SeqCst) != AssetResidency::Resident)
		{
			pAsset->m_useCount.FetchSubtract(1, PAL::MemoryOrder::SeqCst);
			return permit;
		}

		pAsset->m_lastUsedTick.Store(m_useClock.FetchAdd(1, PAL::MemoryOrder::Relaxed), PAL::MemoryOrder::Relaxed);

		permit.pAsset = pAsset;
		permit.pSubMeshes = pAsset->m_submeshes.GetData();
		permit.subMeshCount = u32(pAsset->m_submeshes.GetCount());
		permit.firstVertex = u32(pAsset->m_vertexRange.offset / sizeof(MeshVertex));
		permit.firstIndex = u32(pAsset->m_indexRange.offset / sizeof(u32));

		Terminal::Info(StringOps::GetName(this), "BeginUse {} useCount {}", pAsset->m_ownerEntry.assetId.ToString(), pAsset->m_useCount.Load(PAL::MemoryOrder::SeqCst));

		return permit;
	}

	void MeshAssetStreamer::EndUse(const PermittedMeshData& permit)
	{
		permit.pAsset->m_lastUsedTick.Store(m_useClock.FetchAdd(1, PAL::MemoryOrder::Relaxed), PAL::MemoryOrder::Relaxed);

		const u32 previous = permit.pAsset->m_useCount.FetchSubtract(1, PAL::MemoryOrder::SeqCst);
		Terminal::Assert(previous != 0, StringOps::GetName(this), "Use count underflow, EndUse without BeginUse");
		Terminal::Info(StringOps::GetName(this), "EndUse {} useCount {}", permit.pAsset->m_ownerEntry.assetId.ToString(), previous - 1);
	}

	b8 MeshAssetStreamer::Evict(MeshAsset* pAsset)
	{
		if (pAsset->m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting, PAL::MemoryOrder::SeqCst) != AssetResidency::Resident)
			return false;

		if (pAsset->m_useCount.Load(PAL::MemoryOrder::SeqCst) != 0)
		{
			pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::SeqCst);
			return false;
		}

		m_vertexArena->Free(pAsset->m_vertexRange);
		m_indexArena->Free(pAsset->m_indexRange);

		Terminal::Info(StringOps::GetName(this), "Evicted {} tick {} vertexUsed {} indexUsed {}",
			pAsset->m_ownerEntry.assetId.ToString(), pAsset->m_lastUsedTick.Load(PAL::MemoryOrder::Relaxed),
			m_vertexArena->GetUsedBytes(), m_indexArena->GetUsedBytes());

		pAsset->m_vertexRange = {};
		pAsset->m_indexRange = {};
		pAsset->m_submeshes.Clear();

		m_resident.Remove(pAsset);
		pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);

		return true;
	}

	b8 MeshAssetStreamer::EvictLeastRecentlyUsed()
	{
		for (usize attempt = 0; attempt < m_resident.GetCount(); ++attempt)
		{
			MeshAsset* pVictim = nullptr;
			u64 oldestTick = 0;

			for (MeshAsset* pAsset : m_resident)
			{
				if (pAsset->m_useCount.Load(PAL::MemoryOrder::SeqCst) != 0)
					continue;

				const u64 tick = pAsset->m_lastUsedTick.Load(PAL::MemoryOrder::Relaxed);

				if (pVictim == nullptr || tick < oldestTick)
				{
					oldestTick = tick;
					pVictim = pAsset;
				}
			}

			if (pVictim == nullptr)
				return false;

			if (Evict(pVictim))
				return true;
		}

		return false;
	}

	b8 MeshAssetStreamer::TryAllocate(usize vertexBytes, usize indexBytes, RHI::GfxBufferRange& vertexRange, RHI::GfxBufferRange& indexRange)
	{
		while (true)
		{
			vertexRange = m_vertexArena->Allocate(vertexBytes, sizeof(MeshVertex));

			if (vertexRange.IsValid())
			{
				indexRange = m_indexArena->Allocate(indexBytes, sizeof(u32));

				if (indexRange.IsValid())
					return true;

				m_vertexArena->Free(vertexRange);
				vertexRange = {};
			}

			if (!EvictLeastRecentlyUsed())
				return false;
		}
	}

	void MeshAssetStreamer::RunLoadAsset(MeshAsset* pAsset)
	{
		const AssetHeader& header = pAsset->m_header;

		if (header.magic != AssetHeader::Magic)
		{
			FailedAssetLog(pAsset, "Magic number doesn't match up!");
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			FailedAssetLog(pAsset, "Versions are not matching!");
			return;
		}

		if (header.propertySize != sizeof(MeshProperties))
		{
			FailedAssetLog(pAsset, "MeshProperties are not matching!");
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		std::string cookPath = pAsset->m_ownerEntry.cookPath;
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			FailedAssetLog(pAsset, "Access request to file has failed!");
			return;
		}

		PAL::FileView view = PAL::File::OpenMap(fileHandl);
		PAL::File::ReleaseAccess(fileHandl);

		if (!view.IsValid())
		{
			FailedAssetLog(pAsset, "Failed to map the cook file");
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			FailedAssetLog(pAsset, "Header offsets point outside of the cook file");
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		MeshProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(MeshProperties));

		if (props.vertexStride != sizeof(MeshVertex) || props.indexStride != sizeof(u32))
		{
			FailedAssetLog(pAsset, "VertexStride or IndexStride is not properly sized");
			PAL::File::CloseMap(view);
			return;
		}

		usize subMeshTable = props.subMeshCount * sizeof(MeshSubMesh);
		usize vertBlock = props.vertexCount * props.vertexStride;
		usize indexBlock = props.indexCount * props.indexStride;

		if (subMeshTable + vertBlock + indexBlock != header.payloadSize)
		{
			FailedAssetLog(pAsset, "Payload size does not match MeshProperties");
			PAL::File::CloseMap(view);
			return;
		}

		const u8* pPayload = view.GetData() + header.payloadOffset;

		List<MeshSubMesh> subMeshes(props.subMeshCount);
		std::memcpy(subMeshes.GetData(), pPayload, subMeshTable);

		for (usize i = 0; i < subMeshes.GetCount(); ++i)
		{
			const MeshSubMesh& subMesh = subMeshes[i];

			const b8 hasValidIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 hasValidVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!hasValidIndices || !hasValidVertices)
			{
				FailedAssetLog(pAsset, "a sub-mesh points outside of its buffers");
				PAL::File::CloseMap(view);
				return;
			}
		}

		const u8* pVertexData = pPayload + subMeshTable;
		const u8* pIndexData = pVertexData + vertBlock;

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_arenaLock);

			RHI::GfxBufferRange vRange = {};
			RHI::GfxBufferRange iRange = {};

			if (!TryAllocate(vertBlock, indexBlock, vRange, iRange))
			{
				Terminal::Warn(StringOps::GetName(this), "Arenas are full and every resident mesh is in use, {} will retry on next request", pAsset->m_ownerEntry.assetId.ToString());
				pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}

			pAsset->m_vertexRange = vRange;
			pAsset->m_indexRange = iRange;
		}

		std::memcpy(m_vertexMap + pAsset->m_vertexRange.offset, pVertexData, vertBlock);
		std::memcpy(m_indexMap + pAsset->m_indexRange.offset, pIndexData, indexBlock);

		PAL::File::CloseMap(view);

		pAsset->m_submeshes = std::move(subMeshes);
		pAsset->m_properties = props;

		{
			ScopedLock lockArena(m_arenaLock);

			m_resident.PushBack(pAsset);
			pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
		}
	}
}