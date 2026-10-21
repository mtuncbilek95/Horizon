#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Scene;
}

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::SceneAsset>(), { ".hscene" }]);
	class EDITOR_API DefaultSceneAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultSceneAssetImporter);
	public:
		DefaultSceneAssetImporter() = default;
		~DefaultSceneAssetImporter();

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		Engine::Scene* m_scene = nullptr;
	};
}