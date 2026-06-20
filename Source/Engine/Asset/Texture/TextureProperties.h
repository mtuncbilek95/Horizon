#pragma once

#include <Runtime/RHI/GfxTypes.h>

#include <Engine/Asset/AssetMetadata.h>

namespace Horizon
{
	struct TextureProperties final : public AssetProperties
	{
		~TextureProperties();

		void Serialize(json& outFile) const final;
		void Deserialize(const json& inFile) final;
		void Import(const std::filesystem::path& path) final;
		void Cook(std::ostream& out) const final;

		u32 width = 0, height = 0, channels = 0;
		GfxTextureType texType = GfxTextureType::Tex2D;
		GfxTextureFormat texFmt = GfxTextureFormat::RGBA8;
		u8* imgRaw = nullptr;
	};
}