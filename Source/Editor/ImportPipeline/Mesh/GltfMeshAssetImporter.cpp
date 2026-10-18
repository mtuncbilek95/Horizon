#include "GltfMeshAssetImporter.h"

#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Engine/Asset/Texture/TextureAsset.h>
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
#include <format>
#include <variant>

namespace Horizon::Editor
{
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

		BuildProductKeys();
		return AssetImportResult::Success;
	}

	void GltfMeshAssetImporter::Release()
	{
		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();
		m_hasBuilt = false;

		m_productKeys.Clear();
		m_bufferPaths.Clear();
		m_asset.reset();
		m_parser.reset();
	}

	usize GltfMeshAssetImporter::GetProductCount() const
	{
		return m_productKeys.GetCount();
	}

	ImportDeclaredProduct GltfMeshAssetImporter::GetProduct(usize index) const
	{
		ImportDeclaredProduct product;
		product.key = m_productKeys[index];
		product.assetType = Reflect::TypeOf<Engine::MeshAsset>();
		return product;
	}

	void GltfMeshAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
		for (const std::string& path : m_bufferPaths)
		{
			ImportDependency dependency;
			dependency.kind = ImportDependencyKind::Data;
			dependency.sourceRelativePath = path;
			outDependencies.PushBack(std::move(dependency));
		}

		const auto* pSettings = static_cast<const MeshAssetImportSettings*>(ctx.pSettings);
		if (!pSettings || !pSettings->m_importMaterials)
			return;

		for (const fastgltf::Image& image : m_asset->images)
		{
			const auto* pUri = std::get_if<fastgltf::sources::URI>(&image.data);
			if (!pUri)
				continue;

			ImportDependency dependency;
			dependency.kind = ImportDependencyKind::Asset;
			dependency.sourceRelativePath = std::string(pUri->uri.path());
			dependency.assetType = Reflect::TypeOf<Engine::TextureAsset>();
			outDependencies.PushBack(std::move(dependency));
		}
	}

	AssetImportResult GltfMeshAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx, product);
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

	AssetImportResult GltfMeshAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx, product);
		if (result != AssetImportResult::Success)
			return result;

		const usize tableSize = m_subMeshes.GetCount() * sizeof(Engine::MeshAssetSubmesh);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_subMeshes.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult GltfMeshAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildMesh(ctx, product);
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

			m_bufferPaths.PushBack(relative);
			buffer.data = std::move(vector);
		}

		return true;
	}

	void GltfMeshAssetImporter::BuildProductKeys()
	{
		m_productKeys.Clear();
		m_productKeys.Reserve(m_asset->meshes.size());

		for (usize i = 0; i < m_asset->meshes.size(); ++i)
		{
			const fastgltf::Mesh& mesh = m_asset->meshes[i];

			std::string key = mesh.name.empty() ? std::format("meshes/{}", i) : std::format("meshes/{}", std::string_view(mesh.name));
			std::string unique = key;
			u32 suffix = 1;

			while (HasProductKey(unique))
				unique = std::format("{}#{}", key, suffix++);

			m_productKeys.PushBack(std::move(unique));
		}
	}

	b8 GltfMeshAssetImporter::HasProductKey(const std::string& key) const
	{
		for (const std::string& existing : m_productKeys)
		{
			if (existing == key)
				return true;
		}

		return false;
	}

	AssetImportResult GltfMeshAssetImporter::BuildMesh(const ImportPipelineContext& ctx, usize product)
	{
		if (m_hasBuilt && m_builtProduct == product)
			return AssetImportResult::Success;

		m_vertices.Clear();
		m_indices.Clear();
		m_subMeshes.Clear();
		m_hasBuilt = false;

		if (product >= m_asset->meshes.size())
			return AssetImportResult::InternalError;

		const auto* pSettings = static_cast<const MeshAssetImportSettings*>(ctx.pSettings);
		const f32 scale = pSettings ? (f32)pSettings->m_importScale : 1.0f;

		const fastgltf::Asset& asset = *m_asset;
		const fastgltf::Mesh& mesh = asset.meshes[product];

		for (const fastgltf::Primitive& primitive : mesh.primitives)
		{
			if (primitive.type != fastgltf::PrimitiveType::Triangles)
			{
				Terminal::Warn(StringOps::GetName(this), "{} has a non-triangle primitive, skipped", m_productKeys[product]);
				continue;
			}

			auto positionIt = primitive.findAttribute("POSITION");
			if (positionIt == primitive.attributes.end())
			{
				Terminal::Error(StringOps::GetName(this), "{} has a primitive without positions", m_productKeys[product]);
				return AssetImportResult::InvalidMeshData;
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
					Engine::MeshVertex& vertex = m_vertices[base + index];
					vertex.position = Math::Vec3f(value.x() * scale, value.y() * scale, value.z() * scale);
					subMesh.boundingBox.Expand(vertex.position);
				});

			auto normalIt = primitive.findAttribute("NORMAL");
			if (normalIt != primitive.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, asset.accessors[normalIt->accessorIndex], [&](fastgltf::math::fvec3 value, usize index)
					{
						m_vertices[base + index].normal = Math::Vec3f(value.x(), value.y(), value.z());
					});
			}

			auto tangentIt = primitive.findAttribute("TANGENT");
			if (tangentIt != primitive.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(asset, asset.accessors[tangentIt->accessorIndex], [&](fastgltf::math::fvec4 value, usize index)
					{
						m_vertices[base + index].tangent = Math::Vec4f(value.x(), value.y(), value.z(), value.w());
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

			m_subMeshes.PushBack(subMesh);
		}

		if (m_subMeshes.GetCount() == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} produced no sub-meshes", m_productKeys[product]);
			return AssetImportResult::NoImportableData;
		}

		m_builtProduct = product;
		m_hasBuilt = true;
		return AssetImportResult::Success;
	}
}