#include "DefaultSceneAssetImporter.h"

#include <Engine/Asset/Scene/SceneProperties.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <cstring>
#include <string_view>

namespace Horizon::Editor
{
	DefaultSceneAssetImporter::~DefaultSceneAssetImporter()
	{
		Release();
	}

	AssetImportResult DefaultSceneAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		if (!ctx.pReflection)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be parsed without a reflection system", ctx.sourcePath);
			return AssetImportResult::InternalError;
		}

		Release();
		m_scene = MakeUnique<Engine::Scene>(ctx.pReflection);

		const std::string_view text(reinterpret_cast<const c8*>(fileBytes.GetData()), fileBytes.GetCount());
		JsonArchiveReader reader(text);

		if (!Asset::SceneSerializer::Deserialize(*m_scene, ctx.pReflection, reader) || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid scene archive", ctx.sourcePath);
			Release();
			return AssetImportResult::CorruptedData;
		}

		return AssetImportResult::Success;
	}

	void DefaultSceneAssetImporter::Release()
	{
		m_scene = nullptr;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		if (!m_scene)
			return AssetImportResult::InternalError;

		Asset::SceneProperties props = {};
		props.entityCount = m_scene->GetEntityCount();

		outBytes.Resize(sizeof(Asset::SceneProperties));
		std::memcpy(outBytes.GetData(), &props, sizeof(Asset::SceneProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		if (!m_scene)
			return AssetImportResult::InternalError;

		BinaryArchiveWriter writer;
		Asset::SceneSerializer::Serialize(*m_scene, ctx.pReflection, writer);

		if (!writer.IsComplete())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} scene could not be serialized to binary", ctx.sourcePath);
			return AssetImportResult::SerializationFailed;
		}

		outBytes = writer.GetBytes();
		return AssetImportResult::Success;
	}
}