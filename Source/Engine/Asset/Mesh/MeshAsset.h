#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Engine/Asset/Mesh/PermittedMeshData.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RHI/Buffer/GfxBufferRange.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API MeshAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(MeshAsset);
		friend class MeshAssetStreamer;
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		void LoadAsync();

		PermittedMeshData BeginUse();
		void EndUse(const PermittedMeshData& permit);

	private:
		SubmitTicket m_ticket = InvalidSubmitTicket;

		List<MeshSubMesh> m_submeshes;

		MeshProperties m_properties;
		RHI::GfxBufferRange m_vertexRange;
		RHI::GfxBufferRange m_indexRange;

		PAL::Atomic<u32> m_useCount = 0;
		PAL::Atomic<u64> m_lastUsedTick = 0;
	};
}