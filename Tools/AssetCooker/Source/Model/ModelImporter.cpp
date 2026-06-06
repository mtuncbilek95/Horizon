#include <Model/ModelImporter.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Horizon::Asset
{
	static void WriteTransform(const aiMatrix4x4& transform, f32 outMatrix[16])
	{
		outMatrix[0] = transform.a1; outMatrix[1] = transform.b1; outMatrix[2] = transform.c1; outMatrix[3] = transform.d1;
		outMatrix[4] = transform.a2; outMatrix[5] = transform.b2; outMatrix[6] = transform.c2; outMatrix[7] = transform.d2;
		outMatrix[8] = transform.a3; outMatrix[9] = transform.b3; outMatrix[10] = transform.c3; outMatrix[11] = transform.d3;
		outMatrix[12] = transform.a4; outMatrix[13] = transform.b4; outMatrix[14] = transform.c4; outMatrix[15] = transform.d4;
	}

	static std::string TexturePath(const aiMaterial* material, aiTextureType type)
	{
		if (material->GetTextureCount(type) == 0)
			return {};
		aiString path;
		if (material->GetTexture(type, 0, &path) == AI_SUCCESS)
			return path.C_Str();
		return {};
	}

	static void ExtractMesh(const aiMesh* mesh, u32 materialIndex, CookedModel& outModel)
	{
		MeshRecord meshRecord{};
		meshRecord.vertexOffset = (u32)outModel.vertices.size();
		meshRecord.vertexCount = mesh->mNumVertices;
		meshRecord.indexOffset = (u32)outModel.indices.size();
		meshRecord.materialIndex = materialIndex;

		aiVector3D boundsMin(0, 0, 0), boundsMax(0, 0, 0);
		if (mesh->mNumVertices > 0) { boundsMin = boundsMax = mesh->mVertices[0]; }

		for (u32 vertexIdx = 0; vertexIdx < mesh->mNumVertices; vertexIdx++)
		{
			CookedVertex vertex{};
			const aiVector3D& position = mesh->mVertices[vertexIdx];
			vertex.position[0] = position.x; vertex.position[1] = position.y; vertex.position[2] = position.z;

			if (mesh->HasNormals())
			{
				const aiVector3D& normal = mesh->mNormals[vertexIdx];
				vertex.normal[0] = normal.x; vertex.normal[1] = normal.y; vertex.normal[2] = normal.z;
			}
			if (mesh->HasTangentsAndBitangents())
			{
				const aiVector3D& tangent = mesh->mTangents[vertexIdx];
				const aiVector3D& bitangent = mesh->mBitangents[vertexIdx];
				const aiVector3D& normal = mesh->mNormals[vertexIdx];
				vertex.tangent[0] = tangent.x; vertex.tangent[1] = tangent.y; vertex.tangent[2] = tangent.z;
				vertex.tangent[3] = ((normal ^ tangent) * bitangent < 0.0f) ? -1.0f : 1.0f;
			}
			if (mesh->HasTextureCoords(0))
			{
				const aiVector3D& texCoord = mesh->mTextureCoords[0][vertexIdx];
				vertex.texCoord[0] = texCoord.x; vertex.texCoord[1] = texCoord.y;
			}
			outModel.vertices.push_back(vertex);

			boundsMin.x = std::min(boundsMin.x, position.x); boundsMin.y = std::min(boundsMin.y, position.y); boundsMin.z = std::min(boundsMin.z, position.z);
			boundsMax.x = std::max(boundsMax.x, position.x); boundsMax.y = std::max(boundsMax.y, position.y); boundsMax.z = std::max(boundsMax.z, position.z);
		}

		for (u32 faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
		{
			const aiFace& face = mesh->mFaces[faceIndex];
			for (u32 cornerIdx = 0; cornerIdx < face.mNumIndices; cornerIdx++)
				outModel.indices.push_back(face.mIndices[cornerIdx]);
		}
		meshRecord.indexCount = (u32)outModel.indices.size() - meshRecord.indexOffset;

		aiVector3D center = (boundsMin + boundsMax) * 0.5f;
		aiVector3D extent = boundsMax - center;
		meshRecord.boundsCenter[0] = center.x; meshRecord.boundsCenter[1] = center.y; meshRecord.boundsCenter[2] = center.z;
		meshRecord.boundsRadius = std::sqrt(extent.x * extent.x + extent.y * extent.y + extent.z * extent.z);

		outModel.meshes.push_back(meshRecord);
	}

	static void ExtractMaterial(const aiMaterial* material, CookedModel& outModel)
	{
		MaterialRecord materialRecord{};

		materialRecord.texture[(u32)MaterialSlot::Albedo] = outModel.AddTexture(TexturePath(material, aiTextureType_BASE_COLOR));
		materialRecord.texture[(u32)MaterialSlot::Normal] = outModel.AddTexture(TexturePath(material, aiTextureType_NORMALS));
		materialRecord.texture[(u32)MaterialSlot::MetalRough] = outModel.AddTexture(TexturePath(material, aiTextureType_METALNESS));
		materialRecord.texture[(u32)MaterialSlot::AmbientOcc] = outModel.AddTexture(TexturePath(material, aiTextureType_AMBIENT_OCCLUSION));
		materialRecord.texture[(u32)MaterialSlot::Emissive] = outModel.AddTexture(TexturePath(material, aiTextureType_EMISSIVE));

		aiColor4D baseColor(1, 1, 1, 1);
		material->Get(AI_MATKEY_BASE_COLOR, baseColor);
		materialRecord.baseColorFactor[0] = baseColor.r; materialRecord.baseColorFactor[1] = baseColor.g;
		materialRecord.baseColorFactor[2] = baseColor.b; materialRecord.baseColorFactor[3] = baseColor.a;

		materialRecord.metallicFactor = 1.0f; material->Get(AI_MATKEY_METALLIC_FACTOR, materialRecord.metallicFactor);
		materialRecord.roughnessFactor = 1.0f; material->Get(AI_MATKEY_ROUGHNESS_FACTOR, materialRecord.roughnessFactor);

		aiColor3D emissive(0, 0, 0);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
		materialRecord.emissiveFactor[0] = emissive.r; materialRecord.emissiveFactor[1] = emissive.g; materialRecord.emissiveFactor[2] = emissive.b;

		materialRecord.alphaCutoff = 0.5f;

		materialRecord.flags = MaterialFlag(0);
		int twoSided = 0;
		if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS && twoSided)
			materialRecord.flags = materialRecord.flags | MaterialFlag::DoubleSided;

		aiString alphaMode;
		if (material->Get("$mat.gltf.alphaMode", 0, 0, alphaMode) == AI_SUCCESS &&
			std::strcmp(alphaMode.C_Str(), "MASK") == 0)
		{
			materialRecord.flags = materialRecord.flags | MaterialFlag::AlphaMask;
			f32 cutoff = 0.5f;
			material->Get("$mat.gltf.alphaCutoff", 0, 0, cutoff);
			materialRecord.alphaCutoff = cutoff;
		}

		outModel.materials.push_back(materialRecord);
	}

	static void WalkNode(const aiNode* node, const aiMatrix4x4& parent, CookedModel& outModel)
	{
		aiMatrix4x4 world = parent * node->mTransformation;

		for (u32 meshIdx = 0; meshIdx < node->mNumMeshes; meshIdx++)
		{
			InstanceRecord instance{};
			instance.meshIndex = node->mMeshes[meshIdx];
			WriteTransform(world, instance.transform);
			outModel.instances.push_back(instance);
		}

		for (u32 childIdx = 0; childIdx < node->mNumChildren; childIdx++)
			WalkNode(node->mChildren[childIdx], world, outModel);
	}

	bool ImportModel(const std::filesystem::path& sourcePath, CookedModel& outModel)
	{
		Assimp::Importer importer;

		const u32 flags =
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality;

		const aiScene* scene = importer.ReadFile(sourcePath.string(), flags);
		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
			return false;

		for (u32 materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++)
			ExtractMaterial(scene->mMaterials[materialIdx], outModel);

		for (u32 meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++)
			ExtractMesh(scene->mMeshes[meshIdx], scene->mMeshes[meshIdx]->mMaterialIndex, outModel);

		WalkNode(scene->mRootNode, aiMatrix4x4(), outModel);

		return true;
	}
}