#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/Material/MaterialAssetProperties.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Data]);
	class ENGINE_API MaterialAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(MaterialAsset);
		friend class MaterialAssetLoader;
	public:
		MaterialAsset() = default;
		~MaterialAsset() = default;

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const Guid& GetShaderId() const { return m_properties.shaderId; }
		const List<MaterialParamValue>& GetValues() const { return m_values; }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;

		MaterialAssetProperties m_properties;
		List<MaterialParamValue> m_values;
	};
}