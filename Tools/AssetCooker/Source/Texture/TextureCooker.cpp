#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Texture/TextureCooker.h>

namespace Horizon::Asset
{
	bool CookTextures(CookedModel& model, const std::filesystem::path& baseDir)
	{
		stbi_set_flip_vertically_on_load(true);

		for (TextureRecord& textureRecord : model.textures)
		{
			std::filesystem::path fullPath = baseDir / textureRecord.path;

			int width = 0, height = 0, channels = 0;
			stbi_uc* pixels = stbi_load(fullPath.string().c_str(), &width, &height, &channels, 4);

			if (!pixels)
			{
				textureRecord.width = textureRecord.height = 1;
				textureRecord.format = 0;
				textureRecord.dataOffset = (u64)model.textureBlob.size();
				textureRecord.dataSize = 4;
				const u8 magenta[4] =
				{
					255,
					0,
					255,
					255
				};
				model.textureBlob.insert(model.textureBlob.end(), magenta, magenta + 4);
				continue;
			}

			const u32 bytes = (u32)width * (u32)height * 4;
			textureRecord.width = (u32)width;
			textureRecord.height = (u32)height;
			textureRecord.format = 0;
			textureRecord.dataOffset = (u64)model.textureBlob.size();
			textureRecord.dataSize = bytes;
			model.textureBlob.insert(model.textureBlob.end(), pixels, pixels + bytes);

			stbi_image_free(pixels);
		}
		return true;
	}
}