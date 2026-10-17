#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class MeshAssetImportSettings : public AssetImportSettings
	{
		HORIZON_TYPE_REFLECT(MeshAssetImportSettings);
	public:
		MeshAssetImportSettings() = default;
		~MeshAssetImportSettings() = default;

		HFIELD();
		b8 m_importMeshes = true;

		HFIELD();
		b8 m_importMaterials = true;

		HFIELD();
		b8 m_importScale = 1.0f;

		HFIELD();
		b8 m_importBones = false;

		HFIELD();
		b8 m_importAnimations = false;
	};
}