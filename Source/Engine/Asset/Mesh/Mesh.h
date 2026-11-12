#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Mesh/MeshUsePermit.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/Memory/RangeAllocator.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Asset
{
	HCLASS(Engine::AssetType[Engine::AssetKind::Streamable]);
	class Mesh : public Engine::AssetObject
	{
		HORIZON_TYPE_REFLECT(Mesh);
		friend class MeshStreamer;
	public:
		Mesh() = default;
		~Mesh() = default;

		void LoadAsync();
		void UnloadAsync();

		MeshUsePermit BeginUse();
		void EndUse(MeshUsePermit& permit);

		const Guid& GetId() const { return m_id; }
		Engine::AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const List<MeshSubmesh>& GetSubmeshes() const { return m_submeshes; }
		List<Engine::AssetHandle<Material>>& GetMaterialSlots() { return m_materialSlots; }

		u32 GetFirstVertex() const { return m_vertexRange.offset / sizeof(MeshVertex); }
		u32 GetFirstIndex() const { return m_indexRange.offset / sizeof(u32); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes);

	private:
		void ReleaseUse();

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<Engine::AssetResidency> m_residency = Engine::AssetResidency::Unloaded;
		Engine::SubmitTicket m_ticket = Engine::InvalidSubmitTicket;

		List<MeshSubmesh> m_submeshes;
		List<Engine::AssetHandle<Material>> m_materialSlots;

		MeshProperties m_properties;
		Range m_vertexRange;
		Range m_indexRange;

		PAL::Atomic<u32> m_useCount = 0;
	};
}