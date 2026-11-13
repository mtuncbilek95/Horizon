#include "FbxMeshAssetImporter.h"

#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/BoundingBox.h>

#include <ufbx.h>

#include <bit>
#include <cmath>
#include <cstring>
#include <format>
#include <unordered_map>
#include <utility>

namespace Horizon::Editor
{
	namespace
	{
		struct MeshInstance
		{
			const ufbx_mesh* pMesh = nullptr;
			const ufbx_node* pNode = nullptr;
		};

		struct PackedVertex
		{
			f32 position[3];
			f32 normal[3];
			f32 tangent[4];
			f32 color[4];
			f32 uv[2];
		};

		List<MeshInstance> CollectMeshInstances(const ufbx_scene& scene, b8 isApplyingTransform)
		{
			List<MeshInstance> instances;

			if (!isApplyingTransform)
			{
				for (usize i = 0; i < scene.meshes.count; ++i)
				{
					MeshInstance instance;
					instance.pMesh = scene.meshes.data[i];
					instances.PushBack(instance);
				}

				return instances;
			}

			for (usize i = 0; i < scene.nodes.count; ++i)
			{
				const ufbx_node* pNode = scene.nodes.data[i];
				if (!pNode->mesh)
					continue;

				MeshInstance instance;
				instance.pMesh = pNode->mesh;
				instance.pNode = pNode;
				instances.PushBack(instance);
			}

			return instances;
		}

		u32 ResolveMaterialSlot(const MeshInstance& instance, u32 partIndex)
		{
			const ufbx_material* pMaterial = nullptr;

			if (instance.pNode && partIndex < instance.pNode->materials.count)
				pMaterial = instance.pNode->materials.data[partIndex];
			else if (partIndex < instance.pMesh->materials.count)
				pMaterial = instance.pMesh->materials.data[partIndex];

			if (!pMaterial)
				return 0;

			return pMaterial->typed_id;
		}

		std::string NormalizePath(const ufbx_string& source)
		{
			std::string path(source.data, source.length);

			for (char& character : path)
			{
				if (character == '\\')
					character = '/';
			}

			return path;
		}

		std::string ResolveTexturePath(const ufbx_texture* pTexture)
		{
			if (!pTexture)
				return std::string();

			return NormalizePath(pTexture->relative_filename);
		}

		Math::Vec3f ToVec3f(const ufbx_vec3& value)
		{
			return Math::Vec3f((f32)value.x, (f32)value.y, (f32)value.z);
		}

		Math::Vec3f NormalizeOr(const Math::Vec3f& value, const Math::Vec3f& fallback)
		{
			const f32 lengthSquared = value | value;
			if (lengthSquared <= 1e-20f)
				return fallback;

			return value / std::sqrt(lengthSquared);
		}

		struct PositionKey
		{
			u32 x;
			u32 y;
			u32 z;

			b8 operator==(const PositionKey& other) const = default;
		};

		struct PositionKeyHash
		{
			usize operator()(const PositionKey& key) const
			{
				usize hash = key.x;
				hash = hash * 0x9e3779b97f4a7c15ull + key.y;
				hash = hash * 0x9e3779b97f4a7c15ull + key.z;
				return hash;
			}
		};

		PositionKey MakePositionKey(const Math::Vec3f& position)
		{
			PositionKey key = {};
			key.x = std::bit_cast<u32>(position.X() + 0.0f);
			key.y = std::bit_cast<u32>(position.Y() + 0.0f);
			key.z = std::bit_cast<u32>(position.Z() + 0.0f);
			return key;
		}
	}

	AssetImportResult FbxMeshAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		ufbx_load_opts options = {};
		options.ignore_animation = true;
		options.ignore_embedded = true;
		options.generate_missing_normals = true;
		options.target_axes = ufbx_axes_right_handed_y_up;
		options.target_unit_meters = 1.0f;
		options.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;

		ufbx_error error = {};
		m_sceneNode.reset(ufbx_load_memory(fileBytes.GetData(), fileBytes.GetCount(), &options, &error));

		if (!m_sceneNode)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be parsed: {}", ctx.sourcePath, std::string(error.description.data, error.description.length));
			return AssetImportResult::CorruptedData;
		}

		if (m_sceneNode->meshes.count == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no meshes", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		return AssetImportResult::Success;
	}

	void FbxMeshAssetImporter::Release()
	{
		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();
		m_hasBuilt = false;

		m_sceneNode.reset();
	}

	AssetImportResult FbxMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		Asset::MeshProperties props = {};
		props.vertexCount = (u32)m_vertices.GetCount();
		props.indexCount = (u32)m_indices.GetCount();
		props.vertexStride = sizeof(Asset::MeshVertex);
		props.indexStride = sizeof(u32);
		props.subMeshCount = (u32)m_subMeshes.GetCount();

		outBytes.Resize(sizeof(Asset::MeshProperties));
		std::memcpy(outBytes.GetData(), &props, sizeof(Asset::MeshProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize tableSize = m_subMeshes.GetCount() * sizeof(Asset::MeshSubmesh);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_subMeshes.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize vertexBlock = m_vertices.GetCount() * sizeof(Asset::MeshVertex);
		const usize indexBlock = m_indices.GetCount() * sizeof(u32);

		outBytes.Resize(vertexBlock + indexBlock);
		std::memcpy(outBytes.GetData(), m_vertices.GetData(), vertexBlock);
		std::memcpy(outBytes.GetData() + vertexBlock, m_indices.GetData(), indexBlock);
		return AssetImportResult::Success;
	}

	AssetImportResult FbxMeshAssetImporter::BuildMesh(const ImportPipelineContext& ctx)
	{
		if (m_hasBuilt)
			return AssetImportResult::Success;

		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();

		const auto* pSettings = static_cast<const MeshAssetImportSettings*>(ctx.pSettings);
		const f32 scale = pSettings ? (f32)pSettings->m_importScale : 1.0f;
		const b8 isApplyingTransform = pSettings && pSettings->m_applyTransform;

		const List<MeshInstance> instances = CollectMeshInstances(*m_sceneNode, isApplyingTransform);

		List<u32> triangleIndices;
		List<PackedVertex> corners;
		List<u32> remap;

		for (const MeshInstance& instance : instances)
		{
			const ufbx_mesh& mesh = *instance.pMesh;
			if (mesh.num_triangles == 0)
				continue;

			ufbx_matrix transform = ufbx_identity_matrix;
			if (instance.pNode)
				transform = instance.pNode->geometry_to_world;

			const ufbx_matrix normalMatrix = ufbx_matrix_for_normals(&transform);
			const b8 isMirrored = ufbx_matrix_determinant(&transform) < 0.0f;

			const b8 hasNormals = mesh.vertex_normal.exists;
			const b8 hasTangents = mesh.vertex_tangent.exists && mesh.vertex_bitangent.exists;
			const b8 hasUvs = mesh.vertex_uv.exists;
			const b8 hasColors = mesh.vertex_color.exists;

			triangleIndices.Resize(mesh.max_face_triangles * 3);

			for (usize p = 0; p < mesh.material_parts.count; ++p)
			{
				const ufbx_mesh_part& part = mesh.material_parts.data[p];
				if (part.num_triangles == 0)
					continue;

				corners.Clear();
				corners.Reserve(part.num_triangles * 3);

				for (usize f = 0; f < part.face_indices.count; ++f)
				{
					const ufbx_face face = mesh.faces.data[part.face_indices.data[f]];
					const u32 triangleCount = ufbx_triangulate_face(triangleIndices.GetData(), triangleIndices.GetCount(), &mesh, face);

					for (u32 c = 0; c < triangleCount * 3; ++c)
					{
						const u32 index = triangleIndices[c];

						const Math::Vec3f position = ToVec3f(ufbx_transform_position(&transform, ufbx_get_vertex_vec3(&mesh.vertex_position, index))) * scale;

						Math::Vec3f normal(0.0f, 1.0f, 0.0f);
						if (hasNormals)
							normal = NormalizeOr(ToVec3f(ufbx_transform_direction(&normalMatrix, ufbx_get_vertex_vec3(&mesh.vertex_normal, index))), normal);

						PackedVertex corner = {};
						corner.position[0] = position.X();
						corner.position[1] = position.Y();
						corner.position[2] = position.Z();
						corner.normal[0] = normal.X();
						corner.normal[1] = normal.Y();
						corner.normal[2] = normal.Z();
						corner.tangent[0] = 1.0f;
						corner.tangent[3] = 1.0f;
						corner.color[0] = 1.0f;
						corner.color[1] = 1.0f;
						corner.color[2] = 1.0f;
						corner.color[3] = 1.0f;

						if (hasTangents)
						{
							const Math::Vec3f tangent = NormalizeOr(ToVec3f(ufbx_transform_direction(&transform, ufbx_get_vertex_vec3(&mesh.vertex_tangent, index))), Math::Vec3f(1.0f, 0.0f, 0.0f));
							const Math::Vec3f bitangent = ToVec3f(ufbx_transform_direction(&transform, ufbx_get_vertex_vec3(&mesh.vertex_bitangent, index)));

							corner.tangent[0] = tangent.X();
							corner.tangent[1] = tangent.Y();
							corner.tangent[2] = tangent.Z();

							if (((normal ^ tangent) | bitangent) < 0.0f)
								corner.tangent[3] = -1.0f;
						}

						if (hasUvs)
						{
							const ufbx_vec2 uv = ufbx_get_vertex_vec2(&mesh.vertex_uv, index);
							corner.uv[0] = (f32)uv.x;
							corner.uv[1] = 1.0f - (f32)uv.y;
						}

						if (hasColors)
						{
							const ufbx_vec4 color = ufbx_get_vertex_vec4(&mesh.vertex_color, index);
							corner.color[0] = (f32)color.x;
							corner.color[1] = (f32)color.y;
							corner.color[2] = (f32)color.z;
							corner.color[3] = (f32)color.w;
						}

						corners.PushBack(corner);
					}
				}

				if (corners.GetCount() == 0)
					continue;

				remap.Resize(corners.GetCount());

				ufbx_vertex_stream stream = {};
				stream.data = corners.GetData();
				stream.vertex_count = corners.GetCount();
				stream.vertex_size = sizeof(PackedVertex);

				ufbx_error error = {};
				const usize vertexCount = ufbx_generate_indices(&stream, 1, remap.GetData(), remap.GetCount(), nullptr, &error);

				if (error.type != UFBX_ERROR_NONE)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} indices could not be generated: {}", ctx.sourcePath, std::string(error.description.data, error.description.length));
					return AssetImportResult::InvalidData;
				}

				const usize base = m_vertices.GetCount();
				m_vertices.Resize(base + vertexCount);

				Asset::MeshSubmesh subMesh = {};
				subMesh.vertexOffset = (u32)base;
				subMesh.vertexCount = (u32)vertexCount;
				subMesh.indexOffset = (u32)m_indices.GetCount();
				subMesh.indexCount = (u32)remap.GetCount();
				subMesh.materialSlotIndex = ResolveMaterialSlot(instance, part.index);
				subMesh.boundingBox = Math::BoundingBox::Empty();

				for (usize v = 0; v < vertexCount; ++v)
				{
					const PackedVertex& corner = corners[v];

					Asset::MeshVertex& vertex = m_vertices[base + v];
					vertex.position = Math::Vec3f(corner.position[0], corner.position[1], corner.position[2]);
					vertex.normal = Math::Vec3f(corner.normal[0], corner.normal[1], corner.normal[2]);
					vertex.tangent = Math::Vec4f(corner.tangent[0], corner.tangent[1], corner.tangent[2], corner.tangent[3]);
					vertex.color = Math::Vec4f(corner.color[0], corner.color[1], corner.color[2], corner.color[3]);
					vertex.uv = Math::Vec2f(corner.uv[0], corner.uv[1]);

					subMesh.boundingBox.Expand(vertex.position);
				}

				const usize indexBase = m_indices.GetCount();
				m_indices.Resize(indexBase + remap.GetCount());
				std::memcpy(m_indices.GetData() + indexBase, remap.GetData(), remap.GetCount() * sizeof(u32));

				if (isMirrored)
				{
					const u32 indexEnd = subMesh.indexOffset + subMesh.indexCount;

					for (u32 i = subMesh.indexOffset; i + 2 < indexEnd; i += 3)
						std::swap(m_indices[i + 1], m_indices[i + 2]);
				}

				if (!hasNormals || (pSettings && pSettings->m_smoothNormals))
					GenerateSmoothNormals(subMesh);

				GenerateTangents(subMesh, hasTangents);

				m_subMeshes.PushBack(subMesh);
			}
		}

		if (m_subMeshes.GetCount() == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} produced no sub-meshes", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		m_hasBuilt = true;
		return AssetImportResult::Success;
	}

	void FbxMeshAssetImporter::GenerateSmoothNormals(const Asset::MeshSubmesh& subMesh)
	{
		std::unordered_map<PositionKey, Math::Vec3f, PositionKeyHash> sums;
		sums.reserve(subMesh.vertexCount);

		for (u32 i = 0; i + 2 < subMesh.indexCount; i += 3)
		{
			const Math::Vec3f& p0 = m_vertices[subMesh.vertexOffset + m_indices[subMesh.indexOffset + i]].position;
			const Math::Vec3f& p1 = m_vertices[subMesh.vertexOffset + m_indices[subMesh.indexOffset + i + 1]].position;
			const Math::Vec3f& p2 = m_vertices[subMesh.vertexOffset + m_indices[subMesh.indexOffset + i + 2]].position;

			const Math::Vec3f faceNormal = (p1 - p0) ^ (p2 - p0);

			sums.try_emplace(MakePositionKey(p0), Math::Vec3f(0.0f, 0.0f, 0.0f)).first->second += faceNormal;
			sums.try_emplace(MakePositionKey(p1), Math::Vec3f(0.0f, 0.0f, 0.0f)).first->second += faceNormal;
			sums.try_emplace(MakePositionKey(p2), Math::Vec3f(0.0f, 0.0f, 0.0f)).first->second += faceNormal;
		}

		for (u32 v = 0; v < subMesh.vertexCount; ++v)
		{
			Asset::MeshVertex& vertex = m_vertices[subMesh.vertexOffset + v];

			auto it = sums.find(MakePositionKey(vertex.position));
			if (it == sums.end())
				continue;

			const f32 lengthSquared = it->second | it->second;
			if (lengthSquared <= 1e-20f)
				continue;

			vertex.normal = it->second / std::sqrt(lengthSquared);
		}
	}

	void FbxMeshAssetImporter::GenerateTangents(const Asset::MeshSubmesh& subMesh, b8 hasSourceTangents)
	{
		List<Math::Vec3f> tangents(subMesh.vertexCount, Math::Vec3f(0.0f, 0.0f, 0.0f));
		List<Math::Vec3f> bitangents(subMesh.vertexCount, Math::Vec3f(0.0f, 0.0f, 0.0f));

		if (!hasSourceTangents)
		{
			for (u32 i = 0; i + 2 < subMesh.indexCount; i += 3)
			{
				const u32 i0 = m_indices[subMesh.indexOffset + i];
				const u32 i1 = m_indices[subMesh.indexOffset + i + 1];
				const u32 i2 = m_indices[subMesh.indexOffset + i + 2];

				const Asset::MeshVertex& v0 = m_vertices[subMesh.vertexOffset + i0];
				const Asset::MeshVertex& v1 = m_vertices[subMesh.vertexOffset + i1];
				const Asset::MeshVertex& v2 = m_vertices[subMesh.vertexOffset + i2];

				const Math::Vec3f edge1 = v1.position - v0.position;
				const Math::Vec3f edge2 = v2.position - v0.position;

				const f32 du1 = v1.uv.X() - v0.uv.X();
				const f32 dv1 = v1.uv.Y() - v0.uv.Y();
				const f32 du2 = v2.uv.X() - v0.uv.X();
				const f32 dv2 = v2.uv.Y() - v0.uv.Y();

				const f32 determinant = du1 * dv2 - du2 * dv1;
				if (std::abs(determinant) <= 1e-12f)
					continue;

				const f32 inverse = 1.0f / determinant;
				const Math::Vec3f tangent = (edge1 * dv2 - edge2 * dv1) * inverse;
				const Math::Vec3f bitangent = (edge2 * du1 - edge1 * du2) * inverse;

				tangents[i0] += tangent;
				tangents[i1] += tangent;
				tangents[i2] += tangent;

				bitangents[i0] += bitangent;
				bitangents[i1] += bitangent;
				bitangents[i2] += bitangent;
			}
		}

		for (u32 v = 0; v < subMesh.vertexCount; ++v)
		{
			Asset::MeshVertex& vertex = m_vertices[subMesh.vertexOffset + v];
			const Math::Vec3f& normal = vertex.normal;

			Math::Vec3f source = tangents[v];
			if (hasSourceTangents)
				source = Math::Vec3f(vertex.tangent.X(), vertex.tangent.Y(), vertex.tangent.Z());

			Math::Vec3f tangent = source - normal * (normal | source);
			f32 lengthSquared = tangent | tangent;

			if (lengthSquared <= 1e-20f)
			{
				Math::Vec3f axis(1.0f, 0.0f, 0.0f);
				if (std::abs(normal.X()) >= 0.9f)
					axis = Math::Vec3f(0.0f, 1.0f, 0.0f);

				tangent = axis - normal * (normal | axis);
				lengthSquared = tangent | tangent;
			}

			tangent = tangent / std::sqrt(lengthSquared);

			f32 handedness = vertex.tangent.W();
			if (!hasSourceTangents)
			{
				handedness = 1.0f;
				if (((normal ^ tangent) | bitangents[v]) > 0.0f)
					handedness = -1.0f;
			}

			vertex.tangent = Math::Vec4f(tangent.X(), tangent.Y(), tangent.Z(), handedness);
		}
	}

	void FbxMeshAssetImporter::CollectDependencies(const ImportPipelineContext& ctx, List<std::string>& outPaths)
	{
		const ufbx_scene& scene = *m_sceneNode;

		for (usize i = 0; i < scene.texture_files.count; ++i)
		{
			const ufbx_texture_file& file = scene.texture_files.data[i];
			if (file.relative_filename.length == 0)
				continue;

			outPaths.PushBack(NormalizePath(file.relative_filename));
		}
	}

	void FbxMeshAssetImporter::CollectMaterials(const ImportPipelineContext& ctx, List<ImportedMaterial>& outMaterials)
	{
		const ufbx_scene& scene = *m_sceneNode;

		for (usize i = 0; i < scene.materials.count; ++i)
		{
			const ufbx_material& source = *scene.materials.data[i];

			ImportedMaterial material;
			material.name = std::string(source.name.data, source.name.length);

			b8 isNameTaken = material.name.empty();
			for (const ImportedMaterial& other : outMaterials)
			{
				if (other.name == material.name)
					isNameTaken = true;
			}

			if (isNameTaken)
				material.name = std::format("{}_{}", material.name.empty() ? std::string("Material") : material.name, i);

			material.properties.twoSided = source.features.double_sided.enabled;

			Asset::MaterialParameters& parameters = material.parameters;
			parameters.m_occlusionChannel = 0;
			parameters.m_roughnessChannel = 1;
			parameters.m_metallicChannel = 2;

			if (source.pbr.base_color.texture)
			{
				parameters.m_baseColorFactor = Math::Vec4f::One();
				material.baseColorPath = ResolveTexturePath(source.pbr.base_color.texture);
			}
			else if (source.pbr.base_color.has_value)
			{
				const ufbx_vec4 baseColor = source.pbr.base_color.value_vec4;
				const f32 factor = source.pbr.base_factor.has_value ? (f32)source.pbr.base_factor.value_real : 1.0f;
				parameters.m_baseColorFactor = Math::Vec4f((f32)baseColor.x * factor, (f32)baseColor.y * factor, (f32)baseColor.z * factor, (f32)baseColor.w);
			}

			if (source.pbr.roughness.texture)
			{
				parameters.m_roughness = 1.0f;
				material.roughnessPath = ResolveTexturePath(source.pbr.roughness.texture);
			}
			else if (source.pbr.roughness.has_value)
			{
				parameters.m_roughness = (f32)source.pbr.roughness.value_real;
			}

			if (source.pbr.metalness.texture)
			{
				parameters.m_metallic = 1.0f;
				material.metallicPath = ResolveTexturePath(source.pbr.metalness.texture);
			}
			else if (source.pbr.metalness.has_value)
			{
				parameters.m_metallic = (f32)source.pbr.metalness.value_real;
			}

			if (source.pbr.emission_color.texture)
			{
				parameters.m_emissiveFactor = Math::Vec4f::One();
				material.emissivePath = ResolveTexturePath(source.pbr.emission_color.texture);
			}
			else if (source.pbr.emission_color.has_value)
			{
				const ufbx_vec4 emissive = source.pbr.emission_color.value_vec4;
				const f32 factor = source.pbr.emission_factor.has_value ? (f32)source.pbr.emission_factor.value_real : 1.0f;
				parameters.m_emissiveFactor = Math::Vec4f((f32)emissive.x * factor, (f32)emissive.y * factor, (f32)emissive.z * factor, 1.0f);
			}

			if (source.pbr.normal_map.texture)
				material.normalPath = ResolveTexturePath(source.pbr.normal_map.texture);

			if (source.pbr.ambient_occlusion.texture)
				material.occlusionPath = ResolveTexturePath(source.pbr.ambient_occlusion.texture);

			outMaterials.PushBack(material);
		}
	}
}