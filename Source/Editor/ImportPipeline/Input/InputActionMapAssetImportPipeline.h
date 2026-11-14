#pragma once

#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/Input/InputActionMapAssetImportSettings.h>
#include <Engine/Asset/Input/InputActionMap.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfoAttribute[Reflect::TypeOf<Asset::InputActionMap>(), Reflect::TypeOf<InputActionMapAssetImportSettings>(), { ".hinputmap" }]);
	class InputActionMapAssetImportPipeline : public ImportPipeline
	{
		HORIZON_TYPE_REFLECT(InputActionMapAssetImportPipeline);
	public:
		InputActionMapAssetImportPipeline() = default;
		~InputActionMapAssetImportPipeline() = default;

	protected:
		AssetImportResult Validate(const ImportPipelineContext& ctx, const ImportProductBytes& bytes) final;
	};
}
