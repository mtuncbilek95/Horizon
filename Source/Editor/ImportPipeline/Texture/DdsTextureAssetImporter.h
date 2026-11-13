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
	HCLASS(AssetImporterType[Reflect::TypeOf<Asset::Texture>(), { ".dds" }]);
	class DdsTextureAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DdsTextureAssetImporter);
	public:
		DdsTextureAssetImporter() = default;
		~DdsTextureAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		struct SourceMip
		{
			u32 width = 0;
			u32 height = 0;
			u32 rowBytes = 0;
			u32 rowCount = 0;
			List<u8> bytes;
		};

		AssetImportResult BuildTexture(const ImportPipelineContext& ctx);

	private:
		RHI::TextureFormat m_sourceFormat = RHI::TextureFormat::Undefined;
		List<SourceMip> m_sourceMips;

		Asset::TextureProperties m_properties;
		List<Asset::TextureMip> m_mips;
		List<u8> m_payload;
		b8 m_hasBuilt = false;
	};
}