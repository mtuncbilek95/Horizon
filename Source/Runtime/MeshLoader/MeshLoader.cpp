#include <Runtime/MeshLoader/MeshLoader.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stdexcept>

namespace Horizon
{
	static Math::Mat4f ToMat4(const aiMatrix4x4& m)
	{
		return {
			m.a1, m.b1, m.c1, m.d1,
			m.a2, m.b2, m.c2, m.d2,
			m.a3, m.b3, m.c3, m.d3,
			m.a4, m.b4, m.c4, m.d4
		};
	}

	static MeshData ProcessMesh(aiMesh* mesh)
	{
		MeshData data;
		data.name = mesh->mName.C_Str();
		data.materialIndex = mesh->mMaterialIndex;

		for (u32 i = 0; i < mesh->mNumVertices; i++)
		{
			data.geometry.AddPosition({ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }, i);

			if (mesh->HasNormals())
				data.geometry.AddNormal({ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }, i);

			if (mesh->HasTangentsAndBitangents())
			{
				data.geometry.AddTangent({ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z }, i);
				data.geometry.AddBinormal({ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z }, i);
			}

			if (mesh->HasVertexColors(0))
				data.geometry.AddColor({ mesh->mColors[0][i].r, mesh->mColors[0][i].g,
										 mesh->mColors[0][i].b, mesh->mColors[0][i].a }, i);
			else
				data.geometry.AddColor({ 1.f, 1.f, 1.f, 1.f }, i);

			if (mesh->HasTextureCoords(0))
				data.geometry.AddTexCoord({ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }, i);
		}

		std::vector<u32> indices;
		indices.reserve(mesh->mNumFaces * 3);
		for (u32 i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (u32 j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		data.geometry.AddIndex(indices);

		return data;
	}

	static MaterialData ProcessMaterial(aiMaterial* mat, const std::filesystem::path& modelDir)
	{
		MaterialData data;
		data.name = mat->GetName().C_Str();

		auto getTex = [&](aiTextureType type, std::string& out)
			{
				aiString path;
				if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
					out = (modelDir / path.C_Str()).generic_string();
			};

		getTex(aiTextureType_DIFFUSE, data.albedoPath);
		getTex(aiTextureType_NORMALS, data.normalPath);
		getTex(aiTextureType_METALNESS, data.metallicRoughnessPath);
		getTex(aiTextureType_EMISSIVE, data.emissivePath);
		getTex(aiTextureType_AMBIENT_OCCLUSION, data.occlusionPath);

		if (data.metallicRoughnessPath.empty())
			getTex(aiTextureType_UNKNOWN, data.metallicRoughnessPath);

		aiColor4D color;
		if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
			data.albedoFactor = { color.r, color.g, color.b, color.a };

		aiColor3D emissive(0.f);
		if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
			data.emissiveFactor = { emissive.r, emissive.g, emissive.b };

		mat->Get(AI_MATKEY_METALLIC_FACTOR, data.metallicFactor);
		mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, data.roughnessFactor);

		int doubleSided = 0;
		mat->Get(AI_MATKEY_TWOSIDED, doubleSided);
		data.doubleSided = doubleSided != 0;

		return data;
	}

	static NodeData ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshData>& outMeshes, const Math::Mat4f& parentWorld)
	{
		NodeData nodeData;
		nodeData.name = node->mName.C_Str();
		nodeData.localTransform = ToMat4(node->mTransformation);
		nodeData.worldTransform = parentWorld * nodeData.localTransform;

		for (u32 i = 0; i < node->mNumMeshes; i++)
		{
			u32 meshIndex = static_cast<u32>(outMeshes.size());
			outMeshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]]));
			nodeData.meshIndices.push_back(meshIndex);
		}

		for (u32 i = 0; i < node->mNumChildren; i++)
			nodeData.children.push_back(ProcessNode(node->mChildren[i], scene, outMeshes, nodeData.worldTransform));

		return nodeData;
	}

	SceneData MeshLoader::Load(const std::filesystem::path& path)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path.string(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
			aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | 
			aiProcess_ValidateDataStructure);

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
			throw std::runtime_error(std::string("MeshLoader::Load failed: ") + importer.GetErrorString());

		SceneData sceneData;
		const auto modelDir = path.parent_path();

		sceneData.materials.reserve(scene->mNumMaterials);
		for (u32 i = 0; i < scene->mNumMaterials; i++)
			sceneData.materials.push_back(ProcessMaterial(scene->mMaterials[i], modelDir));

		sceneData.rootNode = ProcessNode(scene->mRootNode, scene, sceneData.meshes, Math::Mat4f(1.f));

		return sceneData;
	}
}
