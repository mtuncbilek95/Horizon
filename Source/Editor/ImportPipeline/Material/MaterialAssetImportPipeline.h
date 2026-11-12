#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Material/MaterialAssetImportSettings.h>
#include <Engine/Asset/Material/Material.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Asset::Material>(), Reflect::TypeOf<MaterialAssetImportSettings>(), { ".hmaterial" }]);
	class MaterialAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(MaterialAssetImportPipeline);
	public:
		MaterialAssetImportPipeline() = default;
		~MaterialAssetImportPipeline() = default;

	protected:
		AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) final;
	};
}