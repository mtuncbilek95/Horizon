#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class MaterialAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(MaterialAssetImportSettings);
	public:
		MaterialAssetImportSettings() = default;
		~MaterialAssetImportSettings() = default;
	};
}