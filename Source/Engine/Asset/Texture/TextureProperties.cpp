#include "TextureProperties.h"

#include <stb_image.h>

namespace Horizon
{
	void TextureProperties::Serialize(json& outFile) const
	{

	}

	void TextureProperties::Deserialize(const json& inFile)
	{
	}

	void TextureProperties::Import(const std::filesystem::path& path)
	{
		i32 w = 0, h = 0, c = 0;

		imgRaw = stbi_load(path.string().data(), &w, &h, &c, 4);

		width = u32(w);
		height = u32(h);
		texType = GfxTextureType::Tex2D;

		switch (c)
		{
		case 1:
			texFmt = GfxTextureFormat::R8;
			break;
		case 2:
			texFmt = GfxTextureFormat::RG8;
			break;
		case 3:
			texFmt = GfxTextureFormat::RGBA8;
			break;
		case 4:
			texFmt = GfxTextureFormat::RGBA8;
			break;
		default:
			texFmt = GfxTextureFormat::RGBA8;
			break;
		}

	}

}