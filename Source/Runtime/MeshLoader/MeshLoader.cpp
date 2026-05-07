#include "MeshLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

namespace Horizon
{
	ObjectInfo MeshLoader::LoadFromGltf(const std::filesystem::path& srcPath)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			srcPath.string(),
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));

		ObjectInfo info;
		info.path = srcPath.string();
		info.type = (scene->mNumMeshes == 1) ? ObjectType::Mesh : ObjectType::Scene;
		info.name = srcPath.stem().string();
		info.hasAnimations = scene->mNumAnimations > 0;
		info.animationCount = scene->mNumAnimations;
		info.totalVertices = 0;
		info.totalIndices = 0;

		for (u32 i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			MeshInfo mi;
			mi.name = mesh->mName.C_Str();
			mi.vertexCount = mesh->mNumVertices;
			mi.indexCount = mesh->mNumFaces * 3;
			mi.materialIndex = mesh->mMaterialIndex;
			mi.hasNormals = mesh->HasNormals();
			mi.hasTangents = mesh->HasTangentsAndBitangents();
			mi.hasTexCoords = mesh->HasTextureCoords(0);
			mi.hasVertexColors = mesh->HasVertexColors(0);
			mi.hasBones = mesh->mNumBones > 0;
			mi.boneCount = mesh->mNumBones;
			mi.aabbMin = { mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z };
			mi.aabbMax = { mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z };

			mi.uvChannelCount = 0;
			for (u32 ch = 0; ch < AI_MAX_NUMBER_OF_TEXTURECOORDS; ch++)
			{
				if (mesh->HasTextureCoords(ch))
					mi.uvChannelCount++;
			}

			info.totalVertices += mi.vertexCount;
			info.totalIndices += mi.indexCount;
			info.meshes.push_back(mi);
		}

		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* mat = scene->mMaterials[i];

			MaterialInfo matInfo;
			aiString matName;
			mat->Get(AI_MATKEY_NAME, matName);
			matInfo.name = matName.C_Str();

			auto getTexPath = [&](aiTextureType type) -> std::string
				{
					if (mat->GetTextureCount(type) > 0)
					{
						auto path = srcPath.parent_path();
						aiString name;
						mat->GetTexture(type, 0, &name);
						return (path / name.C_Str()).string();
					}
					return "";
				};

			matInfo.diffuseMap = getTexPath(aiTextureType_DIFFUSE);
			matInfo.normalMap = getTexPath(aiTextureType_NORMALS);
			matInfo.metalnessMap = getTexPath(aiTextureType_METALNESS);
			matInfo.roughnessMap = getTexPath(aiTextureType_DIFFUSE_ROUGHNESS);
			matInfo.emissiveMap = getTexPath(aiTextureType_EMISSIVE);
			matInfo.aoMap = getTexPath(aiTextureType_AMBIENT_OCCLUSION);

			info.materials.push_back(matInfo);
		}

		return info;
	}

	static TextureData LoadTexture(const std::string& path)
	{
		TextureData tex;
		tex.path = path;

		if (path.empty())
			return tex;

		int w, h, ch;
		u8* data = stbi_load(path.c_str(), &w, &h, &ch, 4);
		if (!data)
			throw std::runtime_error("Failed to load texture: " + path);

		tex.width = w;
		tex.height = h;
		tex.channels = 4;
		tex.pixels.assign(data, data + (w * h * 4));
		stbi_image_free(data);

		return tex;
	}

	ObjectData MeshLoader::ExtractFromGltf(const ObjectInfo& info)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			info.path,
			aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			throw std::runtime_error("Assimp error: " + std::string(importer.GetErrorString()));

		ObjectData data;
		data.name = info.name;

		for (u32 i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			Geometry geo(mesh->mNumVertices, mesh->mNumFaces * 3);

			for (u32 v = 0; v < mesh->mNumVertices; v++)
			{
				geo.AddPosition({ mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z }, v);

				if (mesh->HasNormals())
					geo.AddNormal({ mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z }, v);

				if (mesh->HasTangentsAndBitangents())
				{
					geo.AddTangent({ mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z }, v);
					geo.AddBinormal({ mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z }, v);
				}

				if (mesh->HasTextureCoords(0))
					geo.AddTexCoord({ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y }, v);

				if (mesh->HasVertexColors(0))
					geo.AddColor({ mesh->mColors[0][v].r, mesh->mColors[0][v].g, mesh->mColors[0][v].b, mesh->mColors[0][v].a }, v);
			}

			u32 idx = 0;
			for (u32 f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];
				for (u32 j = 0; j < face.mNumIndices; j++)
					geo.AddIndex(face.mIndices[j], idx++);
			}

			data.meshes.push_back(std::move(geo));
		}

		for (u32 i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* mat = scene->mMaterials[i];
			MaterialData matData;

			aiString matName;
			mat->Get(AI_MATKEY_NAME, matName);
			matData.name = matName.C_Str();

			auto getTexPath = [&](aiTextureType type) -> std::string {
				if (mat->GetTextureCount(type) > 0)
				{
					aiString name;
					mat->GetTexture(type, 0, &name);
					std::filesystem::path fullPath = info.path;
					fullPath = fullPath.parent_path() / name.C_Str();
					return fullPath.lexically_normal().string();
				}
				return "";
				};

			matData.diffuseMap = LoadTexture(getTexPath(aiTextureType_DIFFUSE));
			matData.normalMap = LoadTexture(getTexPath(aiTextureType_NORMALS));
			matData.metalnessMap = LoadTexture(getTexPath(aiTextureType_METALNESS));
			matData.roughnessMap = LoadTexture(getTexPath(aiTextureType_DIFFUSE_ROUGHNESS));
			matData.emissiveMap = LoadTexture(getTexPath(aiTextureType_EMISSIVE));
			matData.aoMap = LoadTexture(getTexPath(aiTextureType_AMBIENT_OCCLUSION));

			data.materials.push_back(std::move(matData));
		}

		return data;
	}
}