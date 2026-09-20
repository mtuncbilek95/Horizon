#pragma once

#include <Engine/Asset/AssetObject.h>

#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT MeshAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(MeshAsset);
		friend class MeshAssetStreamer;
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		RHI::GfxBuffer* GetVertexBuffer() const { return m_vertexBuffer; }
		RHI::GfxBuffer* GetIndexBuffer() const { return m_indexBuffer; }

		u32 GetVertexCount() const { return m_vertexCount; }
		u32 GetIndexCount() const { return m_indexCount; }
		u32 GetVertexStride() const { return m_vertexStride; }

	private:
		RHI::GfxBuffer* m_vertexBuffer = nullptr;
		RHI::GfxBuffer* m_indexBuffer = nullptr;

		u32 m_vertexCount = 0;
		u32 m_indexCount = 0;
		u32 m_vertexStride = 0;
	};
}