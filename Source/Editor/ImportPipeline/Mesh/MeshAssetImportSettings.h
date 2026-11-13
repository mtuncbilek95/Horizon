#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Material/Material.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Attributes/HideInInspectorAttribute.h>
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
		b8 m_importMaterials = true;

		HFIELD();
		f32 m_importScale = 1.0f;

		HFIELD();
		b8 m_importBones = false;

		HFIELD();
		b8 m_importAnimations = false;

		HFIELD();
		b8 m_applyTransform = true;

		HFIELD();
		b8 m_smoothNormals = true;

		HFIELD(HideInInspector, AssetRef[Reflect::TypeOf<Asset::Material>()]);
		List<Engine::AssetHandle<Asset::Material>> m_materialSlots;
	};
}