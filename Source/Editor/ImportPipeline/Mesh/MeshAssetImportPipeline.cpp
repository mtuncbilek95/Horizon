#include "MeshAssetImportPipeline.h"

#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void MeshAssetImportPipeline::PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
		// Check the prop just in case
		if (bytes.properties.GetCount() != sizeof(Asset::MeshProperties))
			return;

		Asset::MeshProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Asset::MeshProperties));

		// Check if the table is matching with subMeshCount
		if (bytes.table.GetCount() != props.subMeshCount * sizeof(Asset::MeshSubmesh))
			return;

		auto* pSubMeshes = (Asset::MeshSubmesh*)(bytes.table.GetData());

		Math::BoundingBox mainBounds = Math::BoundingBox::Empty();
		u32 slotCount = 0;
		for (u32 i = 0; i < props.subMeshCount; ++i)
		{
			mainBounds.Expand(pSubMeshes[i].boundingBox);

			if (pSubMeshes[i].materialSlotIndex + 1 > slotCount)
				slotCount = pSubMeshes[i].materialSlotIndex + 1;
		}

		props.mainBounds = mainBounds;
		props.materialSlotCount = slotCount;
		std::memcpy(bytes.properties.GetData(), &props, sizeof(Asset::MeshProperties));

		const auto* pSettings = static_cast<const MeshAssetImportSettings*>(ctx.pSettings);
		const usize subMeshTable = bytes.table.GetCount();

		bytes.table.Resize(subMeshTable + slotCount * sizeof(Guid));

		for (u32 i = 0; i < slotCount; ++i)
		{
			Guid id;

			if (pSettings && i < pSettings->m_materialSlots.GetCount())
				id = pSettings->m_materialSlots[i].GetId();

			std::memcpy(bytes.table.GetData() + subMeshTable + i * sizeof(Guid), &id, sizeof(Guid));
		}
	}

	AssetImportResult MeshAssetImportPipeline::Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes)
	{
		// Check if the properties are matching
		if (bytes.properties.GetCount() != sizeof(Asset::MeshProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} properties block has a wrong size", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		// Get MeshProperties
		Asset::MeshProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Asset::MeshProperties));

		if (props.vertexStride != sizeof(Asset::MeshVertex) || props.indexStride != sizeof(u32))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has a wrong vertex or index stride", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (props.vertexCount == 0 || props.indexCount == 0 || props.subMeshCount == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no geometry", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != props.subMeshCount * sizeof(Asset::MeshSubmesh) + props.materialSlotCount * sizeof(Guid))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} table size does not match subMeshCount and materialSlotCount", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		const usize vertexBlock = (usize)props.vertexCount * props.vertexStride;
		const usize indexBlock = (usize)props.indexCount * props.indexStride;

		if (bytes.binary.GetCount() != vertexBlock + indexBlock)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} payload size does not match its properties", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		// Here comes the data table
		auto* pSubMeshes = (Asset::MeshSubmesh*)(bytes.table.GetData());
		for (u32 i = 0; i < props.subMeshCount; ++i)
		{
			const Asset::MeshSubmesh& subMesh = pSubMeshes[i];

			const b8 validIndices = subMesh.indexCount != 0 && (u64)subMesh.indexOffset + subMesh.indexCount <= props.indexCount;
			const b8 validVertices = subMesh.vertexCount != 0 && (u64)subMesh.vertexOffset + subMesh.vertexCount <= props.vertexCount;

			if (!validIndices || !validVertices)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} sub-mesh {} points outside of its buffers", ctx.sourcePath, i);
				return AssetImportResult::InvalidData;
			}

			if (!subMesh.boundingBox.IsValid())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} sub-mesh {} has an empty bounding box", ctx.sourcePath, i);
				return AssetImportResult::InvalidData;
			}
		}

		return AssetImportResult::Success;
	}
}