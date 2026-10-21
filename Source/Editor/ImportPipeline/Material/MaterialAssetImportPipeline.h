#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Material/MaterialAssetImportSettings.h>
#include <Engine/Asset/Material/MaterialAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Engine::MaterialAsset>(), Reflect::TypeOf<MaterialAssetImportSettings>(), { ".hmaterial" }]);
	class EDITOR_API MaterialAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(MaterialAssetImportPipeline);
	public:
		MaterialAssetImportPipeline() = default;
		~MaterialAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes) final;
	};
}