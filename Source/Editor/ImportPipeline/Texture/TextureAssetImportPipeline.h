#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Texture/TextureAssetImportSettings.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Asset::Texture>(), Reflect::TypeOf<TextureAssetImportSettings>(), { ".png", ".jpg", ".jpeg", ".dds" }]);
	class TextureAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(TextureAssetImportPipeline);
	public:
		TextureAssetImportPipeline() = default;
		~TextureAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) final;
	};
}