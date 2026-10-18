#include "MeshAssetImportPipeline.h"

#include <Engine/Asset/Mesh/MeshAssetProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void MeshAssetImportPipeline::PostProcess(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
		// Check the prop just in case
		if (bytes.properties.GetCount() != sizeof(Engine::MeshAssetProperties))
			return;

		Engine::MeshAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::MeshAssetProperties));

		// Check if the table is matching with subMeshCount
		if (bytes.table.GetCount() != props.subMeshCount * sizeof(Engine::MeshAssetSubmesh))
			return;

		auto* pSubMeshes = (Engine::MeshAssetSubmesh*)(bytes.table.GetData());

		Math::BoundingBox mainBounds = Math::BoundingBox::Empty();
		for (u32 i = 0; i < props.subMeshCount; ++i)
			mainBounds.Expand(pSubMeshes[i].boundingBox);

		props.mainBounds = mainBounds;
		std::memcpy(bytes.properties.GetData(), &props, sizeof(Engine::MeshAssetProperties));
	}

	AssetImportResult MeshAssetImportPipeline::Validate(const ImportDeclaredProduct& product, const ImportProductBytes& bytes)
	{
		// Check if the properties are matching
		if (bytes.properties.GetCount() != sizeof(Engine::MeshAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} properties block has a wrong size", product.key);
			return AssetImportResult::InvalidMeshData;
		}

		// Get MeshAssetProperties
		Engine::MeshAssetProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::MeshAssetProperties));

		if (props.vertexStride != sizeof(Engine::MeshVertex) || props.indexStride != sizeof(u32))
		{
			Terminal::Error(StringOps::GetName(this), "{} has a wrong vertex or index stride", product.key);
			return AssetImportResult::InvalidMeshData;
		}

		if (props.vertexCount == 0 || props.indexCount == 0 || props.subMeshCount == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no geometry", product.key);
			return AssetImportResult::InvalidMeshData;
		}

		if (bytes.table.GetCount() != props.subMeshCount * sizeof(Engine::MeshAssetSubmesh))
		{
			Terminal::Error(StringOps::GetName(this), "{} table size does not match subMeshCount", product.key);
			return AssetImportResult::InvalidMeshData;
		}

		const usize vertexBlock = (usize)props.vertexCount * props.vertexStride;
		const usize indexBlock = (usize)props.indexCount * props.indexStride;

		if (bytes.binary.GetCount() != vertexBlock + indexBlock)
		{
			Terminal::Error(StringOps::GetName(this), "{} payload size does not match its properties", product.key);
			return AssetImportResult::InvalidMeshData;
		}

		// Here comes the data table
		auto* pSubMeshes = (Engine::MeshAssetSubmesh*)(bytes.table.GetData());
		for (u32 i = 0; i < props.subMeshCount; ++i)
		{
			const Engine::MeshAssetSubmesh& subMesh = pSubMeshes[i];

			const b8 validIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 validVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!validIndices || !validVertices)
			{
				Terminal::Error(StringOps::GetName(this), "{} sub-mesh {} points outside of its buffers", product.key, i);
				return AssetImportResult::InvalidMeshData;
			}

			if (!subMesh.boundingBox.IsValid())
			{
				Terminal::Error(StringOps::GetName(this), "{} sub-mesh {} has an empty bounding box", product.key, i);
				return AssetImportResult::InvalidMeshData;
			}
		}

		return AssetImportResult::Success;
	}
}