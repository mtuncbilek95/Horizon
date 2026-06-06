#pragma once

#include <Assets/AssetFormat.h>

#include <vector>
#include <string>
#include <cstdio>

namespace Horizon::Asset
{
	struct CookedModel
	{
		std::vector<CookedVertex> vertices;
		std::vector<u32> indices;
		std::vector<MeshRecord> meshes;
		std::vector<MaterialRecord> materials;
		std::vector<InstanceRecord> instances;
		std::vector<TextureRecord> textures;
		std::vector<u8> textureBlob;

		i32 AddTexture(const std::string& path)
		{
			if (path.empty())
				return -1;

			for (size_t textureIdx = 0; textureIdx < textures.size(); textureIdx++)
				if (path == textures[textureIdx].path)
					return (int32_t)textureIdx;

			TextureRecord textureRecord{};
			std::snprintf(textureRecord.path, sizeof(textureRecord.path), "%s", path.c_str());
			textures.push_back(textureRecord);
			return (i32)(textures.size() - 1);
		}
	};
}