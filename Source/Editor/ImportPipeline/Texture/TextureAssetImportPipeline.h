#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Texture/TextureAssetImportSettings.h>
#include <Engine/Asset/Texture/TextureAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Engine::TextureAsset>(), Reflect::TypeOf<TextureAssetImportSettings>(), { ".png", ".jpg", ".jpeg", ".tga", ".bmp" }]);
	class EDITOR_API TextureAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(TextureAssetImportPipeline);
	public:
		TextureAssetImportPipeline() = default;
		~TextureAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes) final;
	};
}