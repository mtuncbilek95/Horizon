#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Engine::MeshAsset>(), Reflect::TypeOf<MeshAssetImportSettings>(), { ".fbx", ".gltf", ".glb", ".obj" }]);
	class EDITOR_API MeshAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(MeshAssetImportPipeline);
	public:
		MeshAssetImportPipeline() = default;
		~MeshAssetImportPipeline() = default;

	protected:
		void PostProcess(const ImportPipelineContext& ctx, ImportProductBytes& bytes) final;
		AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) final;
	};
}