#include "InputActionMapAssetImportPipeline.h"

#include <Engine/Asset/Input/InputActionMapSerializer.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Editor
{
	AssetImportResult InputActionMapAssetImportPipeline::Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} input action map carries an unexpected properties block", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} input action map carries an unexpected table block", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.binary.GetCount() == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no input action map payload", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		BinaryArchiveReader reader(bytes.binary.GetData(), bytes.binary.GetCount());

		if (reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} payload is not a valid binary archive", ctx.sourcePath);
			return AssetImportResult::CorruptedData;
		}

		reader.BeginObject();

		if (!reader.Key("version") || reader.ReadU64() > Asset::InputActionMapSerializer::Version)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} input action map version is missing or newer than {}", ctx.sourcePath, Asset::InputActionMapSerializer::Version);
			return AssetImportResult::UnsupportedVersion;
		}

		if (!reader.Key("actions"))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} payload carries no actions", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		return AssetImportResult::Success;
	}
}
