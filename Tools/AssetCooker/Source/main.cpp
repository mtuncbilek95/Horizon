#include <Model/ModelImporter.h>
#include <Model/ModelWriter.h>
#include <Texture/TextureCooker.h>

#include <filesystem>
#include <cstdio>
#include <print>

using namespace Horizon::Asset;

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::println("usage: Cooker <input-model> <output.hmodel>\n");
		return 1;
	}

	std::filesystem::path sourcePath = argv[1];
	std::filesystem::path outputPath = argv[2];

	CookedModel model;
	if (!ImportModel(sourcePath, model))
	{
		std::println("[Cooker] import failed: {}\n", sourcePath.string().c_str());
		return 2;
	}

	if (!CookTextures(model, sourcePath.parent_path()))
	{
		std::println("[Cooker] import failed: {}\n", sourcePath.string().c_str());
		return 3;
	}

	if (!WriteModel(outputPath, model))
	{
		std::println("[Cooker] write failed: {}\n", outputPath.string().c_str());
		return 4;
	}

	std::println("[Cooker] OK: {} -> {}\n", sourcePath.string().c_str(), outputPath.string().c_str());
	std::println("  meshes={} materials={} instances={} verts={} indices={} textures={}\n",
		model.meshes.size(), model.materials.size(), model.instances.size(),
		model.vertices.size(), model.indices.size(), model.textures.size());

	return 0;
}