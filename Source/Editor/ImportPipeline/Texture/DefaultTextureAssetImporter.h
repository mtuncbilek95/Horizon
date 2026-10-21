#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Texture/TextureAsset.h>
#include <Engine/Asset/Texture/TextureAssetProperties.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::TextureAsset>(), { ".png", ".jpg", ".jpeg", ".tga", ".bmp" }]);
	class EDITOR_API DefaultTextureAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultTextureAssetImporter);
	public:
		DefaultTextureAssetImporter() = default;
		~DefaultTextureAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		usize GetProductCount() const final;
		ImportDeclaredProduct GetProduct(usize index) const final;
		void GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies) final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;

	private:
		void BuildMipChain(u32 width, u32 height);

	private:
		Engine::TextureAssetProperties m_properties;
		List<Engine::TextureAssetMip> m_mips;
		List<u8> m_payload;
	};
}