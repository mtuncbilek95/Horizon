#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Texture/TextureAssetCompression.h>
#include <Engine/Asset/Texture/TextureAssetUsage.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class TextureAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(TextureAssetImportSettings);
	public:
		TextureAssetImportSettings() = default;
		~TextureAssetImportSettings() = default;

		HFIELD();
		Engine::TextureAssetUsage m_usage = Engine::TextureAssetUsage::Color;

		HFIELD();
		TextureAssetCompression m_compression = TextureAssetCompression::Default;

		HFIELD();
		b8 m_generateMips = true;

		HFIELD();
		u32 m_maxSize = 0;

		HFIELD();
		b8 m_flipGreenChannel = false;
	};
}