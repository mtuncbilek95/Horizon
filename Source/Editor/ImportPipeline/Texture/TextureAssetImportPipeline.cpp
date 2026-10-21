#include "TextureAssetImportPipeline.h"

#include <Engine/Asset/Texture/TextureAssetProperties.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::Editor
{
	void TextureAssetImportPipeline::PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
	}

	AssetImportResult TextureAssetImportPipeline::Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes)
	{
		// Check if the properties are matching
		if (bytes.properties.GetCount() != sizeof(Engine::TextureAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "Texture properties block has a wrong size");
			return AssetImportResult::InvalidData;
		}

		Engine::TextureAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::TextureAssetProperties));

		if (props.format == RHI::GfxTextureFormat::Undefined || props.textureSize.X() == 0 || props.textureSize.Y() == 0 || props.mipCount == 0)
		{
			Terminal::Error(StringOps::GetName(this), "Texture has an undefined format, an empty size or no mips");
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != props.mipCount * sizeof(Engine::TextureAssetMip))
		{
			Terminal::Error(StringOps::GetName(this), "Texture table size does not match mipCount");
			return AssetImportResult::InvalidData;
		}

		if (bytes.binary.GetCount() != props.textureByteSize)
		{
			Terminal::Error(StringOps::GetName(this), "Texture payload size does not match its properties");
			return AssetImportResult::InvalidData;
		}

		auto* pMips = (const Engine::TextureAssetMip*)(bytes.table.GetData());
		for (u32 i = 0; i < props.mipCount; ++i)
		{
			const Engine::TextureAssetMip& mip = pMips[i];

			const u32 expectedWidth = props.textureSize.X() >> i ? props.textureSize.X() >> i : 1;
			const u32 expectedHeight = props.textureSize.Y() >> i ? props.textureSize.Y() >> i : 1;

			const b8 validSize = mip.width == expectedWidth && mip.height == expectedHeight;
			const b8 validRange = mip.byteSize != 0 && mip.offset + mip.byteSize <= bytes.binary.GetCount();

			if (!validSize || !validRange)
			{
				Terminal::Error(StringOps::GetName(this), "Texture mip {} points outside of the payload or carries a wrong size", i);
				return AssetImportResult::InvalidData;
			}
		}

		return AssetImportResult::Success;
	}
}