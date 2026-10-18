#include "DefaultSceneAssetImporter.h"

namespace Horizon::Editor
{
	AssetImportResult DefaultSceneAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		return AssetImportResult::Success;
	}

	void DefaultSceneAssetImporter::Release()
	{
	}

	usize DefaultSceneAssetImporter::GetProductCount() const
	{
		return 0;
	}

	ImportDeclaredProduct DefaultSceneAssetImporter::GetProduct(usize index) const
	{
		return {};
	}

	void DefaultSceneAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}
}