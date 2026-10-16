#include "GltfMeshAssetImporter.h"

namespace Horizon::Editor
{
	AssetImportResult GltfMeshAssetImporter::CookAsset(Engine::Engine* pEngine, ReadOnlyList<u8> fileBytes, AssetImportSettings* pSettings, List<u8>& outBytes, usize& outPropertySize)
	{
		return AssetImportResult::Success;
	}

	b8 GltfMeshAssetImporter::PatchCook(const AssetImportSettings* pPrevious, const AssetImportSettings* pCurrent, const std::string cookedPath)
	{
		return true;
	}
}