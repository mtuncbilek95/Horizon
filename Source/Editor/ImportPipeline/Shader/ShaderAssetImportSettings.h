#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Material/MaterialBlendMode.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class ShaderAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(ShaderAssetImportSettings);
	public:
		ShaderAssetImportSettings() = default;
		~ShaderAssetImportSettings() = default;

		HFIELD();
		Engine::MaterialBlendMode m_blendMode = Engine::MaterialBlendMode::Opaque;

		HFIELD();
		b8 m_twoSided = false;
	};
}