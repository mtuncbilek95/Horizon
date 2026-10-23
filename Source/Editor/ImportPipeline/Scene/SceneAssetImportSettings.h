#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class SceneAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(SceneAssetImportSettings);
	public:
		SceneAssetImportSettings() = default;
		~SceneAssetImportSettings() = default;

	};
}