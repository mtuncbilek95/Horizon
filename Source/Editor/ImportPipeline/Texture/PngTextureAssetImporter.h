#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Asset/Texture/TextureProperties.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Asset::Texture>(), { ".png", ".jpg", ".jpeg" }]);
	class PngTextureAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(PngTextureAssetImporter);
	public:
		PngTextureAssetImporter() = default;
		~PngTextureAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		AssetImportResult BuildTexture(const ImportPipelineContext& ctx);

	private:
		List<u8> m_sourcePixels;
		u32 m_sourceWidth = 0;
		u32 m_sourceHeight = 0;

		Asset::TextureProperties m_properties;
		List<Asset::TextureMip> m_mips;
		List<u8> m_payload;
		b8 m_hasBuilt = false;
	};
}