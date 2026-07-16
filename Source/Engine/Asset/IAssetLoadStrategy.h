#pragma once

namespace Horizon
{
	class IAsset;
	class AssetEntry;

	class IAssetLoadStrategy
	{
	public:
		virtual ~IAssetLoadStrategy() = default;

		virtual IAsset* Load(const AssetEntry& entry) = 0;
	};
}