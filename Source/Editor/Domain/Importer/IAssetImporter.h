#pragma once

namespace Horizon
{
	class AssetImportContext;

	class IAssetImporter
	{
	public:
		virtual ~IAssetImporter() = default;

		virtual void OnImport(AssetImportContext& context) = 0;
		virtual void OnImportDefault(AssetImportContext& context) = 0;
	};
}