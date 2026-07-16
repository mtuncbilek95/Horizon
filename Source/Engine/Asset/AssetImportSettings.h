#pragma once

namespace Horizon
{
	class AssetImportSettings
	{
	public:
		virtual ~AssetImportSettings() = default;

		virtual std::string_view GetAssetType() const = 0;

		virtual void Serialize(std::vector<u8>& out) const = 0;
		virtual b8 Deserialize(const u8* data, u64 size) = 0;
	};
}