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

		HFIELD();
		b8 m_importMaterials = true;

		HFIELD();
		b8 m_importScale = 1.0f;

		HFIELD();
		b8 m_importBones = false;
	};
}