#include "TextureAssetImportPipeline.h"

#include <Engine/Asset/Texture/TextureAssetProperties.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::Editor
{
	void TextureAssetImportPipeline::PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
	}

	AssetImportResult TextureAssetImportPipeline::Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Engine::TextureAssetProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} properties block has a wrong size", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		Engine::TextureAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::TextureAssetProperties));

		if (props.format == RHI::TextureFormat::Undefined || props.width == 0 || props.height == 0 || props.mipCount == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no valid pixel description", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != props.mipCount * sizeof(Engine::TextureAssetMip))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} table size does not match mipCount", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		auto* pMips = (Engine::TextureAssetMip*)(bytes.table.GetData());
		for (u32 i = 0; i < props.mipCount; ++i)
		{
			const Engine::TextureAssetMip& mip = pMips[i];

			if (mip.width == 0 || mip.height == 0 || mip.offset + mip.size > bytes.binary.GetCount())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} mip {} points outside of the payload", ctx.sourcePath, i);
				return AssetImportResult::InvalidData;
			}
		}

		return AssetImportResult::Success;
	}
}