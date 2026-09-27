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
		Terminal::Assert(m_vertexArena, StringOps::GetName(this), "Big fucked at some places");

		m_vertexMap = (u8*)m_vertexArena->GetBuffer()->Map();

		RHI::GfxBufferArenaDesc indexArenaDesc = {};
		indexArenaDesc.capacity = MibToByte(192ull);
		indexArenaDesc.stride = sizeof(u32);
		indexArenaDesc.memory = RHI::GfxMemoryType::GpuUpload;
		indexArenaDesc.usage = RHI::GfxBufferUsage::Index;
		m_indexArena = m_device->CreateBufferArena(indexArenaDesc);
		Terminal::Assert(m_indexArena, StringOps::GetName(this), "Big fucked at some places");

		m_indexMap = (u8*)m_indexArena->GetBuffer()->Map();
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
			FailedAssetLog(pMeshAsset, "No device! No Vertex Arena! No Index Arena!");
			return;
		}

		pMeshAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Critical, Job([this, pMeshAsset]()
			{
				RunLoadAsset(pMeshAsset);
			}));

		// Nearly impossible but sure
		if (pMeshAsset->m_ticket == InvalidSubmitTicket)
			FailedAssetLog(pMeshAsset, "Job could not be submitted");
	}

	void MeshAssetStreamer::UnloadAsync(AssetObject* pAsset)
	{
		auto* pMeshAsset = static_cast<MeshAsset*>(pAsset);
	}

	void MeshAssetStreamer::RunLoadAsset(MeshAsset* pAsset)
	{
		const AssetHeader& header = pAsset->m_header;

		// Fail checks (Doesn't happen but safe)
		if (header.magic != AssetHeader::Magic)
		{
			FailedAssetLog(pAsset, "Magic number doesn't match up!");
			return;
		}

		// Fail checks (Doesn't happen but safe)
		if (header.version != AssetHeader::Version)
		{
			FailedAssetLog(pAsset, "Versions are not matching!");
			return;
		}

		// Actual reasonable checks
		if (header.propertySize != sizeof(MeshProperties))
		{
			FailedAssetLog(pAsset, "MeshProperties are not matching!");
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		List<u8> propertyBytes;
		List<u8> dataPayload;

		// Read payload then put up your ass.
		std::string cookPath = pAsset->m_ownerEntry.cookPath;
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			FailedAssetLog(pAsset, "Access request to file has failed!");
			return;
		}
		
		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			FailedAssetLog(pAsset, "Failed to read memory file for MeshProperties");
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, dataPayload, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			FailedAssetLog(pAsset, "Failed to read memory file for BinaryData");
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		PAL::File::ReleaseAccess(fileHandl);

		// Check the status and validate some data
		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		MeshProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(MeshProperties));

		// Check if the MeshProp is decent.
		if (props.vertexStride != sizeof(MeshVertex) || props.indexStride != sizeof(u32))
		{
			FailedAssetLog(pAsset, "VertexStride or IndexStride is not properly sized");
			return;
		}

		// Get all the size needed
		usize subMeshTable = props.subMeshCount * sizeof(MeshSubMesh);
		usize vertBlock = props.vertexCount * props.vertexStride;
		usize indexBlock = props.indexCount * props.indexStride;

		List<MeshSubMesh> subMeshes(props.subMeshCount);
		std::memcpy(subMeshes.GetData(), dataPayload.GetData(), subMeshTable);

		// Check if all those subs are there.
		for (usize i = 0; i < subMeshes.GetCount(); ++i)
		{
			const MeshSubMesh& subMesh = subMeshes[i];

			const b8 hasValidIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 hasValidVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!hasValidIndices || !hasValidVertices)
			{
				FailedAssetLog(pAsset, "a sub-mesh points outside of its buffers");
				return;
			}
		}

		const u8* pVertexData = dataPayload.GetData() + subMeshTable;
		const u8* pIndexData = pVertexData + vertBlock;

		// To the arena boys!
		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_arenaLock);
			
			RHI::GfxBufferRange vRange = m_vertexArena->Allocate(vertBlock, sizeof(MeshVertex));
			if (!vRange.IsValid())
			{
				FailedAssetLog(pAsset, "VertexBufferArena is out of space");
				return;
			}

			RHI::GfxBufferRange iRange = m_indexArena->Allocate(indexBlock, sizeof(u32));
			if (!iRange.IsValid())
			{
				FailedAssetLog(pAsset, "IndexBufferArena is out of space");
				m_vertexArena->Free(vRange);
				return;
			}

			pAsset->m_vertexRange = std::move(vRange);
			pAsset->m_indexRange = std::move(iRange);
		}

		std::memcpy(m_vertexMap + pAsset->m_vertexRange.offset, pVertexData, vertBlock);
		std::memcpy(m_indexMap + pAsset->m_indexRange.offset, pIndexData, indexBlock);

		pAsset->m_submeshes = subMeshes;
		pAsset->m_properties = props;
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Relaxed);
	}
}