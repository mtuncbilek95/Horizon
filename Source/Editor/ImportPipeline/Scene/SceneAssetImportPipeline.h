#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Scene/SceneAssetImportSettings.h>
#include <Engine/Asset/Scene/SceneAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Engine::SceneAsset>(), Reflect::TypeOf<SceneAssetImportSettings>(), { ".hscene", ".usd" }]);
	class SceneAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(SceneAssetImportPipeline);
	public:
	};
}