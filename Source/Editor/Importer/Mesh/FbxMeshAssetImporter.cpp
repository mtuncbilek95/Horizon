#include "FbxMeshAssetImporter.h"

#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>

#include <ufbx.h>

#include <cstring>

namespace Horizon::Editor
{
	AssetImportResult FbxMeshAssetImporter::ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr)
	{
		if (inPath.empty())
			return AssetImportResult::EmptyInput;

		ufbx_load_opts opts = {};
		opts.generate_missing_normals = true;

		ufbx_error error = {};
		ufbx_scene* pScene = ufbx_load_file(inPath.c_str(), &opts, &error);

		if (pScene == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be loaded: {}", inPath, error.description.data);

			if (error.type == UFBX_ERROR_FILE_NOT_FOUND)
				return AssetImportResult::FileNotFound;

			return AssetImportResult::InvalidFormat;
		}

		if (pScene->meshes.count == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no mesh", inPath);
			ufbx_free_scene(pScene);
			return AssetImportResult::NoImportableData;
		}

		const ufbx_mesh* pMesh = pScene->meshes.data[0];

		if (pMesh->num_triangles == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} has a mesh without triangles", inPath);
			ufbx_free_scene(pScene);
			return AssetImportResult::InvalidMeshData;
		}

		List<u32> triIndices(pMesh->max_face_triangles * 3);
		List<Engine::MeshVertex> vertices;
		vertices.Reserve(pMesh->num_triangles * 3);

		for (usize faceIndex = 0; faceIndex < pMesh->num_faces; ++faceIndex)
		{
			const ufbx_face face = pMesh->faces.data[faceIndex];
			const u32 triCount = ufbx_triangulate_face(triIndices.GetData(), triIndices.GetCount(), pMesh, face);

			for (u32 i = 0; i < triCount * 3; ++i)
			{
				const u32 index = triIndices[i];

				Engine::MeshVertex vertex;
				vertex.normal = Math::Vec4f::Zero();
				vertex.tangent = Math::Vec4f::Zero();
				vertex.color = Math::Vec4f::One();
				vertex.uv = Math::Vec2f::Zero();

				const ufbx_vec3 position = ufbx_get_vertex_vec3(&pMesh->vertex_position, index);
				vertex.position.Set(static_cast<f32>(position.x), static_cast<f32>(position.y), static_cast<f32>(position.z), 1.f);

				if (pMesh->vertex_normal.exists)
				{
					const ufbx_vec3 normal = ufbx_get_vertex_vec3(&pMesh->vertex_normal, index);
					vertex.normal.Set(static_cast<f32>(normal.x), static_cast<f32>(normal.y), static_cast<f32>(normal.z), 0.f);
				}

				if (pMesh->vertex_tangent.exists)
				{
					const ufbx_vec3 tangent = ufbx_get_vertex_vec3(&pMesh->vertex_tangent, index);
					vertex.tangent.Set(static_cast<f32>(tangent.x), static_cast<f32>(tangent.y), static_cast<f32>(tangent.z), 1.f);
				}

				if (pMesh->vertex_color.exists)
				{
					const ufbx_vec4 color = ufbx_get_vertex_vec4(&pMesh->vertex_color, index);
					vertex.color.Set(static_cast<f32>(color.x), static_cast<f32>(color.y), static_cast<f32>(color.z), static_cast<f32>(color.w));
				}

				if (pMesh->vertex_uv.exists)
				{
					const ufbx_vec2 uv = ufbx_get_vertex_vec2(&pMesh->vertex_uv, index);
					vertex.uv.Set(static_cast<f32>(uv.x), static_cast<f32>(uv.y));
				}

				vertices.PushBack(vertex);
			}
		}

		ufbx_free_scene(pScene);

		List<u32> indices(vertices.GetCount());

		ufbx_vertex_stream stream = {};
		stream.data = vertices.GetData();
		stream.vertex_count = vertices.GetCount();
		stream.vertex_size = sizeof(Engine::MeshVertex);

		const usize uniqueCount = ufbx_generate_indices(&stream, 1, indices.GetData(), indices.GetCount(), nullptr, &error);

		if (error.type != UFBX_ERROR_NONE)
		{
			Terminal::Error(StringOps::GetName(this), "{} index generation failed: {}", inPath, error.description.data);
			return AssetImportResult::InternalError;
		}

		vertices.Resize(uniqueCount);

		Engine::MeshProperties properties = {};
		properties.vertexCount = static_cast<u32>(vertices.GetCount());
		properties.indexCount = static_cast<u32>(indices.GetCount());
		properties.vertexStride = sizeof(Engine::MeshVertex);
		properties.indexStride = sizeof(u32);

		const usize propertyBytes = sizeof(Engine::MeshProperties);
		const usize vertexBytes = vertices.GetCount() * sizeof(Engine::MeshVertex);
		const usize indexBytes = indices.GetCount() * sizeof(u32);

		outByteArr.Resize(propertyBytes + vertexBytes + indexBytes);

		u8* pCursor = outByteArr.GetData();

		std::memcpy(pCursor, &properties, propertyBytes);
		pCursor += propertyBytes;

		std::memcpy(pCursor, vertices.GetData(), vertexBytes);
		pCursor += vertexBytes;

		std::memcpy(pCursor, indices.GetData(), indexBytes);

		return AssetImportResult::Success;
	}
}