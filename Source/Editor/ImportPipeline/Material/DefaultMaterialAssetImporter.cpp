#include "DefaultMaterialAssetImporter.h"

#include <Engine/Asset/Material/MaterialSerializer.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	DefaultMaterialAssetImporter::~DefaultMaterialAssetImporter()
	{
		Release();
	}

	AssetImportResult DefaultMaterialAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		if (!ctx.pReflection)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be parsed without a reflection system", ctx.sourcePath);
			return AssetImportResult::InternalError;
		}

		Release();

		const std::string_view text(reinterpret_cast<const c8*>(fileBytes.GetData()), fileBytes.GetCount());
		JsonArchiveReader reader(text);

		if (!Asset::MaterialSerializer::Deserialize(m_properties, m_parameters, ctx.pReflection, reader) || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid material archive", ctx.sourcePath);
			Release();
			return AssetImportResult::CorruptedData;
		}

		m_parsed = true;
		return AssetImportResult::Success;
	}

	void DefaultMaterialAssetImporter::Release()
	{
		m_properties = Asset::MaterialProperties();
		m_parameters = Asset::MaterialParameters();
		m_parsed = false;
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		if (!m_parsed)
			return AssetImportResult::InternalError;

		Asset::MaterialProperties props = {};
		props.domain = m_properties.domain;
		props.twoSided = m_properties.twoSided;

		outBytes.Resize(sizeof(Asset::MaterialProperties));
		std::memcpy(outBytes.GetData(), &props, sizeof(Asset::MaterialProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		if (!m_parsed)
			return AssetImportResult::InternalError;

		BinaryArchiveWriter writer;

		if (!Asset::MaterialSerializer::Serialize(m_properties, m_parameters, ctx.pReflection, writer) || !writer.IsComplete())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} material could not be serialized to binary", ctx.sourcePath);
			return AssetImportResult::SerializationFailed;
		}

		outBytes = writer.GetBytes();
		return AssetImportResult::Success;
	}
}