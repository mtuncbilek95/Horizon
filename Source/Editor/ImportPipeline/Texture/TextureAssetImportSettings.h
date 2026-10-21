#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
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
		b8 m_compressed = false;
	};
}