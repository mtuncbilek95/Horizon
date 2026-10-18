#include "DefaultSceneAssetImporter.h"

#include <Engine/Asset/Scene/SceneProperties.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
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
			Terminal::Error(StringOps::GetName(this), "{} cannot be parsed without a reflection system", ctx.sourcePath);
			return AssetImportResult::InternalError;
		}

		Release();
		m_scene = Memory::Allocator::Create<Engine::Scene>(Memory::CurrLoc(), ctx.pReflection);

		const std::string_view text(reinterpret_cast<const c8*>(fileBytes.GetData()), fileBytes.GetCount());
		JsonArchiveReader reader(text);

		if (!Engine::SceneSerializer::Deserialize(*m_scene, ctx.pReflection, reader) || reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a valid scene archive", ctx.sourcePath);
			Release();
			return AssetImportResult::CorruptedData;
		}

		return AssetImportResult::Success;
	}

	void DefaultSceneAssetImporter::Release()
	{
		if (!m_scene)
			return;

		Memory::Allocator::Delete(m_scene);
		m_scene = nullptr;
	}

	usize DefaultSceneAssetImporter::GetProductCount() const
	{
		return m_scene ? 1 : 0;
	}

	ImportDeclaredProduct DefaultSceneAssetImporter::GetProduct(usize index) const
	{
		ImportDeclaredProduct product;
		product.assetType = Reflect::TypeOf<Engine::SceneAsset>();
		return product;
	}

	void DefaultSceneAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (!m_scene)
			return AssetImportResult::InternalError;

		Engine::SceneProperties props = {};
		props.entityCount = m_scene->GetEntityCount();

		outBytes.Resize(sizeof(Engine::SceneProperties));
		std::memcpy(outBytes.GetData(), &props, sizeof(Engine::SceneProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultSceneAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (!m_scene)
			return AssetImportResult::InternalError;

		BinaryArchiveWriter writer;
		Engine::SceneSerializer::Serialize(*m_scene, ctx.pReflection, writer);

		if (!writer.IsComplete())
		{
			Terminal::Error(StringOps::GetName(this), "{} scene could not be serialized to binary", ctx.sourcePath);
			return AssetImportResult::SerializationFailed;
		}

		outBytes = writer.GetBytes();
		return AssetImportResult::Success;
	}
}