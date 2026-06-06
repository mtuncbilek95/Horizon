#include <Model/ModelWriter.h>

#include <fstream>

namespace Horizon::Asset
{
	bool WriteModel(const std::filesystem::path& outputPath, const CookedModel& model)
	{
		std::error_code errorCode;
		std::filesystem::create_directories(outputPath.parent_path(), errorCode);

		std::ofstream file(outputPath, std::ios::binary | std::ios::trunc);
		if (!file)
			return false;

		ModelHeader header{};
		header.magic = ModelMagic;
		header.version = ModelVersion;
		header.meshCount = (u32)model.meshes.size();
		header.materialCount = (u32)model.materials.size();
		header.instanceCount = (u32)model.instances.size();
		header.textureCount = (u32)model.textures.size();
		header.vertexCount = (u32)model.vertices.size();
		header.indexCount = (u32)model.indices.size();
		header.vertexStride = sizeof(CookedVertex);
		header.textureDataSize = (u32)model.textureBlob.size();

		auto writeBlob = [&](const void* data, size_t bytes) { if (bytes) file.write((const char*)data, bytes); };

		file.write((const char*)&header, sizeof(header));
		writeBlob(model.meshes.data(), model.meshes.size() * sizeof(MeshRecord));
		writeBlob(model.materials.data(), model.materials.size() * sizeof(MaterialRecord));
		writeBlob(model.instances.data(), model.instances.size() * sizeof(InstanceRecord));
		writeBlob(model.textures.data(), model.textures.size() * sizeof(TextureRecord));
		writeBlob(model.vertices.data(), model.vertices.size() * sizeof(CookedVertex));
		writeBlob(model.indices.data(), model.indices.size() * sizeof(u32));
		writeBlob(model.textureBlob.data(), model.textureBlob.size());


		return (bool)file;
	}
}