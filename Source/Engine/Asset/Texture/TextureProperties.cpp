#include "TextureProperties.h"

#include <stb_image.h>
#include <fstream>

namespace Horizon
{
	TextureProperties::~TextureProperties()
	{
		if (imgRaw)
			stbi_image_free(imgRaw);
	}

	void TextureProperties::Serialize(json& outFile) const
	{
		outFile["width"] = width;
		outFile["height"] = height;
		outFile["textureType"] = u32(texType);
		outFile["textureFormat"] = u32(texFmt);
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
		channels = c == 3 ? 4 : c;
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
			texFmt = GfxTextureFormat::RGB8;
			break;
		default:
			texFmt = GfxTextureFormat::RGBA8;
			break;
		}

	}

	void TextureProperties::Cook(std::ostream& out) const
	{
		const usize byteCount = usize(width) * usize(height) * channels;
		out.write((const c8*)imgRaw, std::streamsize(byteCount));
	}

}