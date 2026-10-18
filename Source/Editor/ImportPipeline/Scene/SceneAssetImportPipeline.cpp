#include "SceneAssetImportPipeline.h"

#include <Engine/Asset/Mesh/MeshAssetProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void SceneAssetImportPipeline::PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
	}

	AssetImportResult SceneAssetImportPipeline::Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes)
	{
		return AssetImportResult::Success;
	}
}