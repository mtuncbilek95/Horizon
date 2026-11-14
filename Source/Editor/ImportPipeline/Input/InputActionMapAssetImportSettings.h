#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class InputActionMapAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(InputActionMapAssetImportSettings);
	public:
		InputActionMapAssetImportSettings() = default;
		~InputActionMapAssetImportSettings() = default;

	};
}
