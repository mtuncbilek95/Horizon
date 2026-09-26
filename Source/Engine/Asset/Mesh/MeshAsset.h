#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Buffer/GfxBufferRange.h>

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

		void LoadAsync();
		void UnloadAsync();

		const List<MeshSubMesh>& GetSubmeshes() const { return m_submeshes; }

		u32 GetFirstVertex() const { return m_vertexRange.offset / sizeof(MeshVertex); }
		u32 GetFirstIndex() const { return m_indexRange.offset / sizeof(u32); }

	private:
		SubmitTicket m_ticket = InvalidSubmitTicket;

		List<MeshSubMesh> m_submeshes;

		MeshProperties m_properties;
		RHI::GfxBufferRange m_vertexRange;
		RHI::GfxBufferRange m_indexRange;
	};
}
