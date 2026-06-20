#pragma once

#include <Runtime/RHI/GfxTypes.h>

#include <Engine/Asset/AssetMetadata.h>

namespace Horizon
{
	struct TextureProperties final : public AssetProperties
	{
		u32 width = 0, height = 0;
		GfxTextureType texType = GfxTextureType::Tex2D;
		GfxTextureFormat texFmt = GfxTextureFormat::RGBA8;
		u8* imgRaw = nullptr;

		void Serialize(json& outFile) const final;
		void Deserialize(const json& inFile) final;
		void Import(const std::filesystem::path& path) final;
	};
}