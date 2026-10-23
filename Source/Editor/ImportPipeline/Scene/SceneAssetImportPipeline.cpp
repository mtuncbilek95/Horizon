#include "SceneAssetImportPipeline.h"

#include <Engine/Asset/Scene/SceneProperties.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>

#include <cstring>

namespace Horizon::Editor
{
	void SceneAssetImportPipeline::PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Engine::SceneProperties))
			return;

		Engine::SceneProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::SceneProperties));

		props.entityCount = 0;

		if (bytes.binary.GetCount() > 0)
		{
			BinaryArchiveReader reader(bytes.binary.GetData(), bytes.binary.GetCount());
			reader.BeginObject();

			if (reader.Key("entities"))
			{
				props.entityCount = static_cast<u32>(reader.BeginArray());
				reader.EndArray();
			}

			reader.EndObject();
		}

		std::memcpy(bytes.properties.GetData(), &props, sizeof(Engine::SceneProperties));
	}

	AssetImportResult SceneAssetImportPipeline::Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes)
	{
		if (bytes.properties.GetCount() != sizeof(Engine::SceneProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} properties block has a wrong size", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		if (bytes.table.GetCount() != 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} scene carries an unexpected table block", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		Engine::SceneProperties props;
		std::memcpy(&props, bytes.properties.GetData(), sizeof(Engine::SceneProperties));

		if (bytes.binary.GetCount() == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no scene payload", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		BinaryArchiveReader reader(bytes.binary.GetData(), bytes.binary.GetCount());

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} payload is not a valid binary archive", ctx.sourcePath);
			return AssetImportResult::CorruptedData;
		}

		reader.BeginObject();

		if (!reader.Key("version") || reader.ReadU64() > Engine::SceneSerializer::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} scene version is missing or newer than {}", ctx.sourcePath, Engine::SceneSerializer::Version);
			return AssetImportResult::UnsupportedVersion;
		}

		if (!reader.Key("entities"))
		{
			Terminal::Error(StringOps::GetName(this), "{} payload carries no entity list", ctx.sourcePath);
			return AssetImportResult::InvalidData;
		}

		const usize entityCount = reader.BeginArray();
		reader.EndArray();
		reader.EndObject();

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} payload could not be walked", ctx.sourcePath);
			return AssetImportResult::CorruptedData;
		}

		if (entityCount != props.entityCount)
		{
			Terminal::Error(StringOps::GetName(this), "{} has {} entities but properties say {}", ctx.sourcePath, entityCount, props.entityCount);
			return AssetImportResult::InvalidData;
		}

		return AssetImportResult::Success;
	}
}