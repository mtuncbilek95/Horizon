#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Scene/SceneAssetImportSettings.h>
#include <Engine/Asset/Scene/Scene.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Asset::Scene>(), Reflect::TypeOf<SceneAssetImportSettings>(), { ".hscene", ".usd" }]);
	class SceneAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(SceneAssetImportPipeline);
	public:
		SceneAssetImportPipeline() = default;
		~SceneAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) final;
	};
}