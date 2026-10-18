#include "GltfMeshAssetImporter.h"

namespace Horizon::Editor
{
	AssetImportResult GltfMeshAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		return AssetImportResult::Success;
	}

	void GltfMeshAssetImporter::Release()
	{
	}

	usize GltfMeshAssetImporter::GetProductCount() const
	{
		return 0;
	}

	ImportDeclaredProduct GltfMeshAssetImporter::GetProduct(usize index) const
	{
		return {};
	}

	void GltfMeshAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult GltfMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult GltfMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult GltfMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}
}