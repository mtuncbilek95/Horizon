#include "MeshAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>
#include <utility>

namespace Horizon::Engine
{
	void MeshAssetStreamer::OnInitialize()
	{
		GraphicsContext* pContext = GetEngine()->RequestContext<GraphicsContext>();

		if (pContext == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, meshes cannot be loaded");
			return;
		}

		m_device = pContext->GetDevice();
	}

	void MeshAssetStreamer::OnFinalize()
	{
		m_device = nullptr;
	}

	AssetObject* MeshAssetStreamer::Load(const AssetEntry& entry)
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
	}
}