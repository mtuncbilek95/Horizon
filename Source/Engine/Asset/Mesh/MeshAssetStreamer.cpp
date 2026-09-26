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
			FailAsset(pMeshAsset, "No device! No Vertex Arena! No Index Arena!");
			return;
		}

		pMeshAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Background, Job([this, pMeshAsset]()
			{
				RunLoadAsset(pMeshAsset);
			}));

		// Nearly impossible but sure
		if (pMeshAsset->m_ticket == InvalidSubmitTicket)
			FailAsset(pMeshAsset, "Job could not be submitted");
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
			FailAsset(pAsset, "Magic number doesn't match up!");
			return;
		}

		// Fail checks (Doesn't happen but safe)
		if (header.version != AssetHeader::Version)
		{
			FailAsset(pAsset, "Versions are not matching!");
			return;
		}

		// Actual reasonable checks
		if (header.propertySize != sizeof(MeshProperties))
		{
			FailAsset(pAsset, "MeshProperties are not matching!");
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
			FailAsset(pAsset, "Access request to file has failed!");
		}
		
		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			FailAsset(pAsset, "Failed to read memory file for MeshProperties");
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, dataPayload, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			FailAsset(pAsset, "Failed to read memory file for BinaryData");
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
			FailAsset(pAsset, "VertexStride or IndexStride is not properly sized");
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
				FailAsset(pAsset, "a sub-mesh points outside of its buffers");
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
				FailAsset(pAsset, "VertexBufferArena is out of space");
				return;
			}

			RHI::GfxBufferRange iRange = m_indexArena->Allocate(indexBlock, sizeof(u32));
			if (!iRange.IsValid())
			{
				FailAsset(pAsset, "IndexBufferArena is out of space");
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

	void MeshAssetStreamer::FailAsset(MeshAsset* pAsset, std::string_view reason)
	{
		Terminal::Error(StringOps::GetName(this), "{} Path: {}", reason, pAsset->m_ownerEntry.assetId.ToString());
		pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
	}

	/*AssetObject* MeshAssetStreamer::Load(const AssetEntry& entry)
	{
		if (m_device == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be loaded without a device", entry.cookedPath);
			return nullptr;
		}

		MeshProperties properties = {};
		List<u8> payload;

		if (!ReadCooked(entry, properties, payload))
			return nullptr;

		const usize subMeshBytes = (usize)properties.subMeshCount * sizeof(MeshSubMesh);
		const usize vertexBytes = (usize)properties.vertexCount * properties.vertexStride;
		const usize indexBytes = (usize)properties.indexCount * properties.indexStride;

		if (properties.vertexStride != sizeof(MeshVertex) || properties.indexStride != sizeof(u32) || subMeshBytes == 0 || vertexBytes == 0 || indexBytes == 0 || payload.GetCount() != subMeshBytes + vertexBytes + indexBytes)
		{
			Terminal::Error(StringOps::GetName(this), "{} has mesh properties that do not match its payload", entry.cookedPath);
			return nullptr;
		}

		List<MeshSubMesh> subMeshes(properties.subMeshCount);
		std::memcpy(subMeshes.GetData(), payload.GetData(), subMeshBytes);

		for (usize i = 0; i < subMeshes.GetCount(); ++i)
		{
			const MeshSubMesh& subMesh = subMeshes[i];

			const b8 hasValidIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= properties.indexCount;
			const b8 hasValidVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= properties.vertexCount;

			if (!hasValidIndices || !hasValidVertices)
			{
				Terminal::Error(StringOps::GetName(this), "{} has submesh {} pointing outside of its buffers", entry.cookedPath, i);
				return nullptr;
			}
		}

		const u8* pVertexData = payload.GetData() + subMeshBytes;
		const u8* pIndexData = pVertexData + vertexBytes;

		RHI::GfxBuffer* pVertexBuffer = CreateFilledBuffer(RHI::GfxBufferUsage::None, pVertexData, vertexBytes, properties.vertexStride);

		if (pVertexBuffer == nullptr)
			return nullptr;

		RHI::GfxBuffer* pIndexBuffer = CreateFilledBuffer(RHI::GfxBufferUsage::Index, pIndexData, indexBytes, properties.indexStride);

		if (pIndexBuffer == nullptr)
		{
			Memory::Allocator::Delete(pVertexBuffer);
			return nullptr;
		}

		MeshAsset* pAsset = Memory::Allocator::Create<MeshAsset>(Memory::CurrLoc());
		pAsset->m_vertexBuffer = pVertexBuffer;
		pAsset->m_indexBuffer = pIndexBuffer;
		pAsset->m_vertexCount = properties.vertexCount;
		pAsset->m_indexCount = properties.indexCount;
		pAsset->m_vertexStride = properties.vertexStride;
		pAsset->m_subMeshes = std::move(subMeshes);

		Bind(pAsset, entry.assetId, AssetResidency::Resident);

		Terminal::Info(StringOps::GetName(this), "{} is resident with {} submeshes, {} vertices and {} indices", entry.assetId.ToString(), properties.subMeshCount, properties.vertexCount, properties.indexCount);

		return pAsset;
	}

	void MeshAssetStreamer::Unload(AssetObject* pObject)
	{
		MeshAsset* pAsset = static_cast<MeshAsset*>(pObject);

		if (pAsset->m_vertexBuffer)
			Memory::Allocator::Delete(pAsset->m_vertexBuffer);

		if (pAsset->m_indexBuffer)
			Memory::Allocator::Delete(pAsset->m_indexBuffer);

		Memory::Allocator::Delete(pAsset);
	}

	b8 MeshAssetStreamer::ReadCooked(const AssetEntry& entry, MeshProperties& outProperties, List<u8>& outPayload)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(entry.cookedPath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::Exclusive);

		if (!request.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be opened", entry.cookedPath);
			return false;
		}

		AssetHeader header = {};
		List<u8> headerBytes;
		List<u8> propertyBytes;

		b8 wasRead = PAL::File::ReadMemory(request, headerBytes, 0, sizeof(AssetHeader)) && headerBytes.GetCount() == sizeof(AssetHeader);

		if (wasRead)
		{
			std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));
			wasRead = header.magic == AssetHeader::Magic && header.version == AssetHeader::Version && header.propertySize == sizeof(MeshProperties) && header.payloadSize != 0;
		}

		if (wasRead)
			wasRead = PAL::File::ReadMemory(request, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize);

		if (wasRead)
			wasRead = PAL::File::ReadMemory(request, outPayload, header.payloadOffset, header.payloadOffset + header.payloadSize);

		PAL::File::ReleaseAccess(request);

		if (!wasRead || propertyBytes.GetCount() != sizeof(MeshProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a valid mesh binary", entry.cookedPath);
			return false;
		}

		std::memcpy(&outProperties, propertyBytes.GetData(), sizeof(MeshProperties));

		return true;
	}

	RHI::GfxBuffer* MeshAssetStreamer::CreateFilledBuffer(RHI::GfxBufferUsage usage, const u8* pData, usize size, u32 stride)
	{
		RHI::GfxBufferDesc desc = {};
		desc.usage = usage;
		desc.memory = RHI::GfxMemoryType::GpuUpload;
		desc.size = size;
		desc.stride = stride;

		RHI::GfxBuffer* pBuffer = m_device->CreateBuffer(desc);

		if (pBuffer == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "A mesh buffer of {} bytes could not be created", size);
			return nullptr;
		}

		void* pMapped = pBuffer->Map();

		if (pMapped == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "A mesh buffer of {} bytes could not be mapped", size);
			Memory::Allocator::Delete(pBuffer);
			return nullptr;
		}

		std::memcpy(pMapped, pData, size);
		pBuffer->Unmap();

		return pBuffer;
	}*/
}