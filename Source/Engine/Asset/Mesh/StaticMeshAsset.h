#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/Asset/Mesh/StaticMeshVertex.h>
#include <Engine/Asset/Mesh/SubMesh.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT StaticMeshAsset final : public Asset
	{
		HORIZON_TYPE_REFLECT(StaticMeshAsset);
		friend class StaticMeshLoadStrategy;
	public:
		StaticMeshAsset() = default;
		~StaticMeshAsset() = default;

		const List<StaticMeshVertex>& GetVertices() const { return m_vertices; }
		const List<u32>& GetIndices() const { return m_indices; }
		const List<SubMesh>& GetSubMeshes() const { return m_subMeshes; }

		RHI::GfxBuffer* GetVertexBuffer() const { return m_vertexBuffer; }
		RHI::GfxBuffer* GetIndexBuffer() const { return m_indexBuffer; }

		u32 GetVertexBufferIndex() const { return m_vertexBuffer ? m_vertexBuffer->GetShaderView() : kInvalid32; }
		u32 GetIndexBufferIndex() const { return m_indexBuffer ? m_indexBuffer->GetShaderView() : kInvalid32; }

	private:
		List<StaticMeshVertex> m_vertices;
		List<u32> m_indices;
		List<SubMesh> m_subMeshes;

		RHI::GfxBuffer* m_vertexBuffer = nullptr;
		RHI::GfxBuffer* m_indexBuffer = nullptr;
	};
}