#pragma once

#include <Runtime/Data/Geometry.h>

namespace Horizon
{
	enum class ObjectType
	{
		Mesh,
		Scene
	};

	struct MeshInfo
	{
		std::string name;
		u32 vertexCount = 0;
		u32 indexCount = 0;
		u32 materialIndex = 0;

		b8 hasNormals = false;
		b8 hasTangents = false;
		b8 hasTexCoords = false;
		b8 hasVertexColors = false;
		u32 uvChannelCount = 0;

		b8 hasBones = false;
		u32 boneCount = 0;

		Math::Vec3f aabbMin{ 0.0f };
		Math::Vec3f aabbMax{ 0.0f };
	};

	struct MaterialInfo
	{
		std::string name;

		std::string diffuseMap;
		std::string normalMap;
		std::string metalnessMap;
		std::string roughnessMap;
		std::string emissiveMap;
		std::string aoMap;
	};

	struct ObjectInfo
	{
		ObjectType type;
		std::string name;
		std::string path;

		std::vector<MeshInfo> meshes;
		std::vector<MaterialInfo> materials;

		b8 hasAnimations = false;
		u32 animationCount = 0;

		u32 totalVertices = 0;
		u32 totalIndices = 0;
	};

	struct TextureData
	{
		std::string path;
		std::vector<u8> pixels;
		int width = 0;
		int height = 0;
		int channels = 0;
	};

	struct MaterialData
	{
		std::string name;
		TextureData diffuseMap;
		TextureData normalMap;
		TextureData metalnessMap;
		TextureData roughnessMap;
		TextureData emissiveMap;
		TextureData aoMap;
	};

	struct ObjectData
	{
		std::string name;
		std::vector<Geometry> meshes;
		std::vector<MaterialData> materials;
	};

	struct MeshLoader
	{
		static ObjectInfo LoadFromGltf(const std::filesystem::path& srcPath);
		static ObjectData ExtractFromGltf(const ObjectInfo& info);
	};
}