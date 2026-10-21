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

	AssetImportResult FbxMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		return AssetImportResult::Success;
	}
}