#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Runtime/Math/Vec4f.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HCLASS();
	class MaterialParameters
	{
		HORIZON_TYPE_REFLECT(MaterialParameters);
	public:
		HFIELD();
		Math::Vec4f m_baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };

		HFIELD();
		Math::Vec4f m_emissiveFactor = { 0.0f, 0.0f, 0.0f, 1.0f };

		HFIELD();
		f32 m_metallic = 0.0f;

		HFIELD();
		f32 m_roughness = 0.5f;

		HFIELD();
		f32 m_normalScale = 1.0f;

		HFIELD();
		f32 m_alphaCutoff = 0.5f;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_baseColorTexture;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_normalTexture;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_occlusionTexture;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_roughnessTexture;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_metallicTexture;

		HFIELD(AssetRef[Reflect::TypeOf<Texture>()]);
		Engine::AssetHandle<Texture> m_emissiveTexture;

		HFIELD();
		u32 m_occlusionChannel = 0;

		HFIELD();
		u32 m_roughnessChannel = 0;

		HFIELD();
		u32 m_metallicChannel = 0;
	};
}