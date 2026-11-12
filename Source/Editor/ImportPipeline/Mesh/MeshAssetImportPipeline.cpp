#include "MeshAssetImportPipeline.h"

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Engine/Asset/Material/MaterialSerializer.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	namespace
	{
		std::string GetFileStem(const std::string& path)
		{
			const usize separator = path.find_last_of("/\\");
			const std::string name = separator == std::string::npos ? path : path.substr(separator + 1);

			return name.substr(0, name.rfind('.'));
		}

		std::string SanitizeFileStem(const std::string& name)
		{
			std::string result = name;

			for (char& character : result)
			{
				const b8 isInvalid = character == '.' || character == '/' || character == '\\' || character == ':' || character == '*'
					|| character == '?' || character == '"' || character == '<' || character == '>' || character == '|';

				if (isInvalid)
					character = '_';
			}

			return result;
		}

		b8 IsStemTaken(const ImportPipelineContext& ctx, const std::string& stem)
		{
			if (StringHelper::CheckIfEquals(GetFileStem(ctx.sourcePath), stem))
				return true;

			for (const ImportDependency& dependency : ctx.dependencies)
			{
				if (StringHelper::CheckIfEquals(GetFileStem(dependency.path), stem))
					return true;
			}

			return false;
		}

		Guid FindDependencyId(const ImportPipelineContext& ctx, const std::string& path)
		{
			for (const ImportDependency& dependency : ctx.dependencies)
			{
				if (dependency.path == path)
					return dependency.id;
			}

			return Guid();
		}
	}

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

		List<Guid> importedMaterials;

		if (pSettings && pSettings->m_importMaterials && ctx.pImporter)
			GenerateMaterials(ctx, importedMaterials);

		bytes.table.Resize(subMeshTable + slotCount * sizeof(Guid));

		for (u32 i = 0; i < slotCount; ++i)
		{
			Guid id;

			if (pSettings && i < pSettings->m_materialSlots.GetCount())
				id = pSettings->m_materialSlots[i].GetId();

			if (!id.IsValid() && i < importedMaterials.GetCount())
				id = importedMaterials[i];

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

	void MeshAssetImportPipeline::GenerateMaterials(const ImportPipelineContext& ctx, List<Guid>& outIds)
	{
		List<ImportedMaterial> materials;
		ctx.pImporter->CollectMaterials(ctx, materials);

		for (ImportedMaterial& material : materials)
		{
			material.parameters.m_baseColorTexture.SetId(FindDependencyId(ctx, material.baseColorPath));
			material.parameters.m_normalTexture.SetId(FindDependencyId(ctx, material.normalPath));
			material.parameters.m_occlusionTexture.SetId(FindDependencyId(ctx, material.occlusionPath));
			material.parameters.m_roughnessTexture.SetId(FindDependencyId(ctx, material.roughnessPath));
			material.parameters.m_metallicTexture.SetId(FindDependencyId(ctx, material.metallicPath));
			material.parameters.m_emissiveTexture.SetId(FindDependencyId(ctx, material.emissivePath));

			std::string stem = SanitizeFileStem(material.name);

			if (IsStemTaken(ctx, stem))
				stem += "_Material";

			const std::string path = ctx.sourceFolder + "/" + stem + ".hmaterial";
			const std::string metaPath = path + ".hmeta";

			Guid id;

			if (PAL::File::Exists(metaPath))
				id = ReadMaterialId(metaPath);
			else if (PAL::File::Exists(path))
				Terminal::Warn(StringHelper::GetTypeName(this), "{} exists without a meta, its slot is left empty", path);
			else
				id = CreateMaterial(ctx, path, metaPath, material);

			outIds.PushBack(id);
		}
	}

	Guid MeshAssetImportPipeline::ReadMaterialId(const std::string& metaPath)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!request.IsValid())
		{
			Terminal::Warn(StringHelper::GetTypeName(this), "{} could not be opened for reading", metaPath);
			return Guid();
		}

		std::string text;
		PAL::File::ReadString(request, text);
		PAL::File::ReleaseAccess(request);

		JsonArchiveReader reader(text);
		reader.BeginObject();

		Guid id;
		if (reader.Key("id"))
			id = Guid(reader.ReadString());

		return id;
	}

	Guid MeshAssetImportPipeline::CreateMaterial(const ImportPipelineContext& ctx, const std::string& path, const std::string& metaPath, const ImportedMaterial& material)
	{
		JsonArchiveWriter writer;

		if (!Asset::MaterialSerializer::Serialize(material.properties, material.parameters, ctx.pReflection, writer))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be serialized", path);
			return Guid();
		}

		const Guid id = Guid::Generate();

		JsonArchiveWriter metaWriter;
		metaWriter.BeginObject();
		metaWriter.Key("id");
		metaWriter.WriteString(id.ToString());
		metaWriter.Key("settings");
		metaWriter.BeginObject();
		metaWriter.EndObject();
		metaWriter.EndObject();

		if (!PAL::File::Create(metaPath))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be created", metaPath);
			return Guid();
		}

		PAL::FileAccessRequest metaRequest = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasMetaWritten = PAL::File::WriteString(metaRequest, metaWriter.ToString());

		PAL::File::ReleaseAccess(metaRequest);

		if (!wasMetaWritten)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be written", metaPath);
			return Guid();
		}

		if (!PAL::File::Create(path))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be created", path);
			PAL::File::Delete(metaPath);
			return Guid();
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be written", path);
			PAL::File::Delete(metaPath);
			return Guid();
		}

		return id;
	}
}