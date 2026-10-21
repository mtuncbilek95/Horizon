#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Shader/ShaderAssetImportSettings.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Engine::ShaderAsset>(), Reflect::TypeOf<ShaderAssetImportSettings>(), { ".hlsl" }]);
	class EDITOR_API ShaderAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(ShaderAssetImportPipeline);
	public:
		ShaderAssetImportPipeline() = default;
		~ShaderAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes) final;
	};
}