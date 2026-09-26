#include "FbxMeshAssetImporter.h"

#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Engine/Asset/Mesh/MeshVertex.h>

#include <ufbx.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Horizon::Editor
{
	static ufbx_vec3 NormalizeDirection(const ufbx_vec3& direction)
	{
		const ufbx_real length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

		if (length <= 0)
			return direction;

		ufbx_vec3 result = {};
		result.x = direction.x / length;
		result.y = direction.y / length;
		result.z = direction.z / length;
		return result;
	}

	AssetImportResult FbxMeshAssetImporter::ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr)
	{
		if (inPath.empty())
			return AssetImportResult::EmptyInput;

		ufbx_load_opts opts = {};
		opts.generate_missing_normals = true;
		opts.target_axes = ufbx_axes_right_handed_y_up;
		opts.target_unit_meters = 1.f;

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

		usize totalCornerCount = 0;

		for (usize nodeIndex = 0; nodeIndex < pScene->nodes.count; ++nodeIndex)
		{
			const ufbx_mesh* pMesh = pScene->nodes.data[nodeIndex]->mesh;

			if (pMesh != nullptr)
				totalCornerCount += pMesh->num_triangles * 3;
		}

		List<Engine::MeshVertex> vertices;
		List<u32> indices;
		List<Engine::MeshSubMesh> subMeshes;

		vertices.Reserve(totalCornerCount);
		indices.Reserve(totalCornerCount);

		List<u32> triIndices;
		List<Engine::MeshVertex> partVertices;
		List<u32> partIndices;

		for (usize nodeIndex = 0; nodeIndex < pScene->nodes.count; ++nodeIndex)
		{
			const ufbx_node* pNode = pScene->nodes.data[nodeIndex];
			const ufbx_mesh* pMesh = pNode->mesh;

			if (pMesh == nullptr || pMesh->num_triangles == 0)
				continue;

			const ufbx_matrix toWorld = pNode->geometry_to_world;
			const ufbx_matrix toWorldNormal = ufbx_matrix_for_normals(&toWorld);
			const b8 isMirrored = ufbx_matrix_determinant(&toWorld) < 0;

			triIndices.Resize(pMesh->max_face_triangles * 3);

			for (usize partIndex = 0; partIndex < pMesh->material_parts.count; ++partIndex)
			{
				const ufbx_mesh_part& part = pMesh->material_parts.data[partIndex];

				if (part.num_triangles == 0)
					continue;

				partVertices.Clear();
				partVertices.Reserve(part.num_triangles * 3);

				f32 minX = 0.f, minY = 0.f, minZ = 0.f;
				f32 maxX = 0.f, maxY = 0.f, maxZ = 0.f;

				for (usize faceIndex = 0; faceIndex < part.num_faces; ++faceIndex)
				{
					const ufbx_face face = pMesh->faces.data[part.face_indices.data[faceIndex]];
					const u32 triCount = ufbx_triangulate_face(triIndices.GetData(), triIndices.GetCount(), pMesh, face);

					for (u32 tri = 0; tri < triCount; ++tri)
					{
						for (u32 corner = 0; corner < 3; ++corner)
						{
							const u32 index = triIndices[tri * 3 + (isMirrored ? 2 - corner : corner)];

							Engine::MeshVertex vertex;
							vertex.normal = Math::Vec3f::Zero();
							vertex.tangent = Math::Vec4f::Zero();
							vertex.color = Math::Vec4f::One();
							vertex.uv = Math::Vec2f::Zero();

							const ufbx_vec3 position = ufbx_transform_position(&toWorld, ufbx_get_vertex_vec3(&pMesh->vertex_position, index));
							const f32 posX = static_cast<f32>(position.x);
							const f32 posY = static_cast<f32>(position.y);
							const f32 posZ = static_cast<f32>(position.z);
							vertex.position.Set(posX, posY, posZ);

							if (partVertices.IsEmpty())
							{
								minX = maxX = posX;
								minY = maxY = posY;
								minZ = maxZ = posZ;
							}
							else
							{
								minX = std::min(minX, posX);
								minY = std::min(minY, posY);
								minZ = std::min(minZ, posZ);
								maxX = std::max(maxX, posX);
								maxY = std::max(maxY, posY);
								maxZ = std::max(maxZ, posZ);
							}

							if (pMesh->vertex_normal.exists)
							{
								const ufbx_vec3 normal = NormalizeDirection(ufbx_transform_direction(&toWorldNormal, ufbx_get_vertex_vec3(&pMesh->vertex_normal, index)));
								vertex.normal.Set(static_cast<f32>(normal.x), static_cast<f32>(normal.y), static_cast<f32>(normal.z));
							}

							if (pMesh->vertex_tangent.exists)
							{
								const ufbx_vec3 tangent = NormalizeDirection(ufbx_transform_direction(&toWorld, ufbx_get_vertex_vec3(&pMesh->vertex_tangent, index)));
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

							partVertices.PushBack(vertex);
						}
					}
				}

				if (partVertices.IsEmpty())
					continue;

				partIndices.Resize(partVertices.GetCount());

				ufbx_vertex_stream stream = {};
				stream.data = partVertices.GetData();
				stream.vertex_count = partVertices.GetCount();
				stream.vertex_size = sizeof(Engine::MeshVertex);

				const usize uniqueCount = ufbx_generate_indices(&stream, 1, partIndices.GetData(), partIndices.GetCount(), nullptr, &error);

				if (error.type != UFBX_ERROR_NONE)
				{
					Terminal::Error(StringOps::GetName(this), "{} index generation failed on node {}: {}", inPath, pNode->name.data, error.description.data);
					ufbx_free_scene(pScene);
					return AssetImportResult::InternalError;
				}

				const usize vertexBase = vertices.GetCount();
				const usize indexBase = indices.GetCount();

				vertices.Resize(vertexBase + uniqueCount);
				std::memcpy(vertices.GetData() + vertexBase, partVertices.GetData(), uniqueCount * sizeof(Engine::MeshVertex));

				indices.Resize(indexBase + partIndices.GetCount());
				std::memcpy(indices.GetData() + indexBase, partIndices.GetData(), partIndices.GetCount() * sizeof(u32));

				Engine::MeshSubMesh subMesh = {};
				subMesh.indexOffset = static_cast<u32>(indexBase);
				subMesh.indexCount = static_cast<u32>(partIndices.GetCount());
				subMesh.vertexOffset = static_cast<u32>(vertexBase);
				subMesh.vertexCount = static_cast<u32>(uniqueCount);
				subMesh.boundsMin.Set(minX, minY, minZ);
				subMesh.boundsMax.Set(maxX, maxY, maxZ);

				subMeshes.PushBack(subMesh);
			}
		}

		ufbx_free_scene(pScene);

		if (subMeshes.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "{} has no mesh with triangles", inPath);
			return AssetImportResult::InvalidMeshData;
		}

		Engine::MeshProperties properties = {};
		properties.vertexCount = static_cast<u32>(vertices.GetCount());
		properties.indexCount = static_cast<u32>(indices.GetCount());
		properties.vertexStride = sizeof(Engine::MeshVertex);
		properties.indexStride = sizeof(u32);
		properties.subMeshCount = static_cast<u32>(subMeshes.GetCount());

		const usize propertyBytes = sizeof(Engine::MeshProperties);
		const usize subMeshBytes = subMeshes.GetCount() * sizeof(Engine::MeshSubMesh);
		const usize vertexBytes = vertices.GetCount() * sizeof(Engine::MeshVertex);
		const usize indexBytes = indices.GetCount() * sizeof(u32);

		outByteArr.Resize(propertyBytes + subMeshBytes + vertexBytes + indexBytes);

		u8* pCursor = outByteArr.GetData();

		std::memcpy(pCursor, &properties, propertyBytes);
		pCursor += propertyBytes;

		std::memcpy(pCursor, subMeshes.GetData(), subMeshBytes);
		pCursor += subMeshBytes;

		std::memcpy(pCursor, vertices.GetData(), vertexBytes);
		pCursor += vertexBytes;

		std::memcpy(pCursor, indices.GetData(), indexBytes);

		Terminal::Info(StringOps::GetName(this), "{} imported with {} submeshes, {} vertices and {} indices", inPath, subMeshes.GetCount(), vertices.GetCount(), indices.GetCount());

		return AssetImportResult::Success;
	}
}
