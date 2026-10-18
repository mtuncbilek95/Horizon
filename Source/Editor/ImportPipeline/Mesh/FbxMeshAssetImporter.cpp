#include "FbxMeshAssetImporter.h"

namespace Horizon::Editor
{
	AssetImportResult FbxMeshAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		return AssetImportResult::Success;
	}

	void FbxMeshAssetImporter::Release()
	{
	}

	usize FbxMeshAssetImporter::GetProductCount() const
	{
		return 0;
	}

	ImportDeclaredProduct FbxMeshAssetImporter::GetProduct(usize index) const
	{
		return {};
	}

	void FbxMeshAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult FbxMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}
}