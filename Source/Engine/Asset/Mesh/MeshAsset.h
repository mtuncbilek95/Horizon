#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Mesh/MeshAssetProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Buffer/GfxBufferRange.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Streamable]);
	class ENGINE_API MeshAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(MeshAsset);
		friend class MeshAssetStreamer;
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		void LoadAsync();
		void UnloadAsync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const List<MeshAssetSubmesh>& GetSubmeshes() const { return m_submeshes; }

		u32 GetFirstVertex() const { return m_vertexRange.offset / sizeof(MeshVertex); }
		u32 GetFirstIndex() const { return m_indexRange.offset / sizeof(u32); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes);

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;
		SubmitTicket m_ticket = InvalidSubmitTicket;

		List<MeshAssetSubmesh> m_submeshes;

		MeshAssetProperties m_properties;
		RHI::GfxBufferRange m_vertexRange;
		RHI::GfxBufferRange m_indexRange;
	};
}