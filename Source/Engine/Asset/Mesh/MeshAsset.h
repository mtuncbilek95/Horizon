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
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		RHI::GfxBuffer* GetVertexBuffer() const { return m_vertexBuffer; }
		RHI::GfxBuffer* GetIndexBuffer() const { return m_indexBuffer; }

		u32 GetIndexCount() const { return m_indexCount; }

	private:
		RHI::GfxBuffer* m_vertexBuffer;
		RHI::GfxBuffer* m_indexBuffer;

		u32 m_indexCount = 0;
	};
}