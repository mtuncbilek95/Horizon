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

		Memory::Allocator::Delete(m_indexArena);
		Memory::Allocator::Delete(m_vertexArena);
	}

	void MeshAssetStreamer::LoadAsync(AssetObject* pAsset)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(pAsset);

		if (m_device == nullptr || m_vertexArena == nullptr || m_indexArena == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "No device! No Vertex Arena! No Index Arena!", pMeshAsset->m_ownerEntry.assetId.ToString());
			pMeshAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pMeshAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Critical, Job([this, pMeshAsset]()
			{
				RunLoadAsset(pMeshAsset);
			}));

		if (pMeshAsset->m_ticket == InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Job could not be submitted", pMeshAsset->m_ownerEntry.assetId.ToString());
			pMeshAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void MeshAssetStreamer::UnloadAsync(AssetObject* pAsset)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(pAsset);
	}

	void MeshAssetStreamer::RunLoadAsset(MeshAsset* pAsset)
	{
		const AssetHeader& header = pAsset->m_header;

		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (header.propertySize != sizeof(MeshProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "MeshProperties are not matching!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		std::string cookPath = pAsset->m_ownerEntry.cookPath;
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		PAL::FileView view = PAL::File::OpenMap(fileHandl);
		PAL::File::ReleaseAccess(fileHandl);

		if (!view.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to map the cook file", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Header offsets point outside of the cook file", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		MeshProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(MeshProperties));

		if (props.vertexStride != sizeof(MeshVertex) || props.indexStride != sizeof(u32))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "VertexStride or IndexStride is not properly sized", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::CloseMap(view);
			return;
		}

		usize subMeshTable = props.subMeshCount * sizeof(MeshSubMesh);
		usize vertBlock = props.vertexCount * props.vertexStride;
		usize indexBlock = props.indexCount * props.indexStride;

		if (subMeshTable + vertBlock + indexBlock != header.payloadSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Payload size does not match MeshProperties", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
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
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "a sub-mesh points outside of its buffers", pAsset->m_ownerEntry.assetId.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}
		}

		const u8* pVertexData = pPayload + subMeshTable;
		const u8* pIndexData = pVertexData + vertBlock;

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_arenaLock);

			RHI::GfxBufferRange vRange = m_vertexArena->Allocate(vertBlock, sizeof(MeshVertex));
			if (!vRange.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "VertexBufferArena is out of space", pAsset->m_ownerEntry.assetId.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::CloseMap(view);
				return;
			}

			RHI::GfxBufferRange iRange = m_indexArena->Allocate(indexBlock, sizeof(u32));
			if (!iRange.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "IndexBufferArena is out of space", pAsset->m_ownerEntry.assetId.ToString());
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
