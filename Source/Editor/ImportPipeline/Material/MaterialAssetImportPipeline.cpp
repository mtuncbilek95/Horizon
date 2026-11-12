#include "MaterialAssetImportPipeline.h"

#include <Engine/Asset/Material/MaterialProperties.h>
#include <Engine/Asset/Material/MaterialSerializer.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Editor
{
	AssetImportResult MaterialAssetImportPipeline::Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Asset::MaterialProperties))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} properties block has a wrong size", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} material carries an unexpected table block", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.binary.GetCount() == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no material payload", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		Asset::MaterialProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Asset::MaterialProperties));

		if (props.domain > Asset::MaterialDomain::Transparent)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} properties carry an unknown domain", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		BinaryArchiveReader reader(bytes.binary.GetData(), bytes.binary.GetCount());

		if (reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} payload is not a valid binary archive", ctx.sourcePath);
			return AssetImportResult::CorruptedData;
		}

		reader.BeginObject();

		if (!reader.Key("version") || reader.ReadU64() > Asset::MaterialSerializer::Version)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} material version is missing or newer than {}", ctx.sourcePath, Asset::MaterialSerializer::Version);
			return AssetImportResult::UnsupportedVersion;
		}

		if (!reader.Key("parameters"))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} payload carries no parameters", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		return AssetImportResult::Success;
	}
}