#pragma once

#include <Engine/Asset/AssetType.h>

#include <vector>
#include <unordered_map>

namespace Horizon
{
	class IAssetLoader;

	struct AssetTypeDesc
	{
		AssetTypeId id = AssetTypeId::Invalid;
		const c8* name = nullptr;
		IAssetLoader* loader = nullptr;
		const void* defaultAsset = nullptr;
	};

	class AssetTypeRegistry
	{
	public:
		u16 Register(const AssetTypeDesc& desc);
		u16 IndexOf(AssetTypeId id) const;

		const AssetTypeDesc& At(u16 index) const { return m_types[index]; }

	private:
		std::vector<AssetTypeDesc> m_types;
		std::unordered_map<AssetTypeId, u16, AssetTypeIdHash> m_byId;
	};
}