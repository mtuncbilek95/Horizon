#include "GltfMeshAssetImporter.h"

#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/BoundingBox.h>
#include <Runtime/PAL/File/File.h>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <cstring>
#include <filesystem>
#include <utility>
#include <variant>

namespace Horizon::Editor
{
	namespace
	{
		struct MeshInstance
		{
			usize meshIndex = 0;
			fastgltf::math::fmat4x4 transform;
		};

		List<MeshInstance> CollectMeshInstances(fastgltf::Asset& asset, b8 isApplyingTransform)
		{
			List<MeshInstance> instances;

			if (!isApplyingTransform || asset.scenes.empty())
			{
				for (usize i = 0; i < asset.meshes.size(); ++i)
				{
					MeshInstance instance;
					instance.meshIndex = i;
					instances.PushBack(instance);
				}

				return instances;
			}

			const usize sceneIndex = asset.defaultScene.value_or(0);
			fastgltf::iterateSceneNodes(asset, sceneIndex, fastgltf::math::fmat4x4(), [&](fastgltf::Node& node, const fastgltf::math::fmat4x4& matrix)
				{
					if (!node.meshIndex.has_value())
						return;

					MeshInstance instance;
					instance.meshIndex = node.meshIndex.value();
					instance.transform = matrix;
					instances.PushBack(instance);
				});

			return instances;
		}
	}

	AssetImportResult GltfMeshAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		auto data = fastgltf::GltfDataBuffer::FromBytes(reinterpret_cast<const std::byte*>(fileBytes.GetData()), fileBytes.GetCount());
		if (data.error() != fastgltf::Error::None)
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a readable glTF buffer", ctx.sourcePath);
			return AssetImportResult::InvalidFormat;
		}

		m_parser.reset(Memory::Allocator::Create<fastgltf::Parser>(Memory::CurrLoc()));

		auto asset = m_parser->loadGltf(data.get(), std::filesystem::path(ctx.sourceFolder), fastgltf::Options::None);
		if (asset.error() != fastgltf::Error::None)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be parsed: {}", ctx.sourcePath, fastgltf::getErrorMessage(asset.error()));
			return AssetImportResult::CorruptedData;
		}

		m_asset.reset(Memory::Allocator::Create<fastgltf::Asset>(Memory::CurrLoc(), std::move(asset.get())));

		if (!LoadExternalBuffers(ctx))
			return AssetImportResult::ExternalDataRequired;

		if (m_asset->meshes.empty())
		{
			Terminal::Error(StringOps::GetName(this), "{} has no meshes", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		return AssetImportResult::Success;
	}

	void GltfMeshAssetImporter::Release()
	{
		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();
		m_hasBuilt = false;

		m_asset.reset();
		m_parser.reset();
	}

	AssetImportResult GltfMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		Engine::MeshAssetProperties props = {};
		props.vertexCount = (u32)m_vertices.GetCount();
		props.indexCount = (u32)m_indices.GetCount();
		props.vertexStride = sizeof(Engine::MeshVertex);
		props.indexStride = sizeof(u32);
		props.subMeshCount = (u32)m_subMeshes.GetCount();

		outBytes.Resize(sizeof(Engine::MeshAssetProperties));
		std::memcpy(outBytes.GetData(), &props, sizeof(Engine::MeshAssetProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult GltfMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize tableSize = m_subMeshes.GetCount() * sizeof(Engine::MeshAssetSubmesh);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_subMeshes.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult GltfMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize vertexBlock = m_vertices.GetCount() * sizeof(Engine::MeshVertex);
		const usize indexBlock = m_indices.GetCount() * sizeof(u32);

		outBytes.Resize(vertexBlock + indexBlock);
		std::memcpy(outBytes.GetData(), m_vertices.GetData(), vertexBlock);
		std::memcpy(outBytes.GetData() + vertexBlock, m_indices.GetData(), indexBlock);
		return AssetImportResult::Success;
	}

	b8 GltfMeshAssetImporter::LoadExternalBuffers(const ImportPipelineContext& ctx)
	{
		for (fastgltf::Buffer& buffer : m_asset->buffers)
		{
			auto* pUri = std::get_if<fastgltf::sources::URI>(&buffer.data);
			if (!pUri)
				continue;

			const std::string relative = std::string(pUri->uri.path());
			const std::string path = ctx.sourceFolder + "/" + relative;

			PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
			if (!request.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} external buffer {} is missing", ctx.sourcePath, relative);
				return false;
			}

			List<u8> bytes;
			const b8 wasRead = PAL::File::ReadMemory(request, bytes);
			PAL::File::ReleaseAccess(request);

			if (!wasRead || bytes.GetCount() < pUri->fileByteOffset)
			{
				Terminal::Error(StringOps::GetName(this), "{} external buffer {} could not be read", ctx.sourcePath, relative);
				return false;
			}

			const usize count = bytes.GetCount() - pUri->fileByteOffset;

			fastgltf::sources::Vector vector;
			vector.bytes.resize(count);
			std::memcpy(vector.bytes.data(), bytes.GetData() + pUri->fileByteOffset, count);
			vector.mimeType = fastgltf::MimeType::GltfBuffer;

			buffer.data = std::move(vector);
		}

		return true;
	}

	AssetImportResult GltfMeshAssetImporter::BuildMesh(const ImportPipelineContext& ctx)
	{
		if (m_hasBuilt)
			return AssetImportResult::Success;

		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();

		const auto* pSettings = static_cast<const MeshAssetImportSettings*>(ctx.pSettings);
		const f32 scale = pSettings ? (f32)pSettings->m_importScale : 1.0f;
		const b8 isApplyingTransform = pSettings && pSettings->m_applyTransform;

		const List<MeshInstance> instances = CollectMeshInstances(*m_asset, isApplyingTransform);
		const fastgltf::Asset& asset = *m_asset;

		for (const MeshInstance& instance : instances)
		{
			const fastgltf::math::fmat4x4& transform = instance.transform;
			const fastgltf::math::fmat3x3 linear = fastgltf::math::fmat3x3(transform);
			const fastgltf::math::fmat3x3 normalMatrix = fastgltf::math::transpose(fastgltf::math::inverse(linear));
			const b8 isMirrored = fastgltf::math::determinant(linear) < 0.0f;

			const fastgltf::Mesh& mesh = asset.meshes[instance.meshIndex];

			for (const fastgltf::Primitive& primitive : mesh.primitives)
			{
				if (primitive.type != fastgltf::PrimitiveType::Triangles)
				{
					Terminal::Warn(StringOps::GetName(this), "{} has a non-triangle primitive, skipped", ctx.sourcePath);
					continue;
				}

				auto positionIt = primitive.findAttribute("POSITION");
				if (positionIt == primitive.attributes.end())
				{
					Terminal::Error(StringOps::GetName(this), "{} has a primitive without positions", ctx.sourcePath);
					return AssetImportResult::InvalidData;
				}

				const fastgltf::Accessor& positions = asset.accessors[positionIt->accessorIndex];
				if (positions.count == 0)
					continue;

				const usize base = m_vertices.GetCount();
				m_vertices.Resize(base + positions.count);

				Engine::MeshAssetSubmesh subMesh = {};
				subMesh.vertexOffset = (u32)base;
				subMesh.vertexCount = (u32)positions.count;
				subMesh.indexOffset = (u32)m_indices.GetCount();
				subMesh.materialSlotIndex = (u32)primitive.materialIndex.value_or(0);
				subMesh.boundingBox = Math::BoundingBox::Empty();

				for (usize v = base; v < m_vertices.GetCount(); ++v)
				{
					Engine::MeshVertex& vertex = m_vertices[v];
					vertex.normal = Math::Vec3f(0.0f, 1.0f, 0.0f);
					vertex.tangent = Math::Vec4f(1.0f, 0.0f, 0.0f, 1.0f);
					vertex.color = Math::Vec4f::One();
					vertex.uv = Math::Vec2f::Zero();
				}

				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positions, [&](fastgltf::math::fvec3 value, usize index)
					{
						const fastgltf::math::fvec4 world = transform * fastgltf::math::fvec4(value.x(), value.y(), value.z(), 1.0f);

						Engine::MeshVertex& vertex = m_vertices[base + index];
						vertex.position = Math::Vec3f(world.x() * scale, world.y() * scale, world.z() * scale);
						subMesh.boundingBox.Expand(vertex.position);
					});

				auto normalIt = primitive.findAttribute("NORMAL");
				if (normalIt != primitive.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, asset.accessors[normalIt->accessorIndex], [&](fastgltf::math::fvec3 value, usize index)
						{
							const fastgltf::math::fvec3 normal = fastgltf::math::normalize(normalMatrix * value);
							m_vertices[base + index].normal = Math::Vec3f(normal.x(), normal.y(), normal.z());
						});
				}

				auto tangentIt = primitive.findAttribute("TANGENT");
				if (tangentIt != primitive.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, asset.accessors[tangentIt->accessorIndex], [&](fastgltf::math::fvec4 value, usize index)
						{
							const fastgltf::math::fvec3 tangent = fastgltf::math::normalize(linear * fastgltf::math::fvec3(value.x(), value.y(), value.z()));
							const f32 handedness = isMirrored ? -value.w() : value.w();
							m_vertices[base + index].tangent = Math::Vec4f(tangent.x(), tangent.y(), tangent.z(), handedness);
						});
				}

				auto uvIt = primitive.findAttribute("TEXCOORD_0");
				if (uvIt != primitive.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, asset.accessors[uvIt->accessorIndex], [&](fastgltf::math::fvec2 value, usize index)
						{
							m_vertices[base + index].uv = Math::Vec2f(value.x(), value.y());
						});
				}

				auto colorIt = primitive.findAttribute("COLOR_0");
				if (colorIt != primitive.attributes.end())
				{
					const fastgltf::Accessor& colors = asset.accessors[colorIt->accessorIndex];

					if (colors.type == fastgltf::AccessorType::Vec4)
					{
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, colors, [&](fastgltf::math::fvec4 value, usize index)
							{
								m_vertices[base + index].color = Math::Vec4f(value.x(), value.y(), value.z(), value.w());
							});
					}
					else if (colors.type == fastgltf::AccessorType::Vec3)
					{
						fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, colors, [&](fastgltf::math::fvec3 value, usize index)
							{
								m_vertices[base + index].color = Math::Vec4f(value.x(), value.y(), value.z(), 1.0f);
							});
					}
				}

				if (primitive.indicesAccessor.has_value())
				{
					const fastgltf::Accessor& indices = asset.accessors[primitive.indicesAccessor.value()];
					const usize indexBase = m_indices.GetCount();

					m_indices.Resize(indexBase + indices.count);
					subMesh.indexCount = (u32)indices.count;

					fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, indices, [&](std::uint32_t value, usize index)
						{
							m_indices[indexBase + index] = value;
						});
				}
				else
				{
					subMesh.indexCount = subMesh.vertexCount;
					m_indices.Reserve(m_indices.GetCount() + subMesh.vertexCount);

					for (u32 i = 0; i < subMesh.vertexCount; ++i)
						m_indices.PushBack(i);
				}

				if (subMesh.indexCount == 0)
					continue;

				if (isMirrored)
				{
					const u32 indexEnd = subMesh.indexOffset + subMesh.indexCount;

					for (u32 i = subMesh.indexOffset; i + 2 < indexEnd; i += 3)
						std::swap(m_indices[i + 1], m_indices[i + 2]);
				}

				m_subMeshes.PushBack(subMesh);
			}
		}

		if (m_subMeshes.GetCount() == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} produced no sub-meshes", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		m_hasBuilt = true;
		return AssetImportResult::Success;
	}
}