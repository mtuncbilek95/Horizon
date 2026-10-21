#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Material/MaterialParamType.h>
#include <Engine/Asset/Texture/TextureAsset.h>
#include <Runtime/Math/Vec4f.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	HCLASS();
	struct EDITOR_API MaterialParamEntry
	{
		HORIZON_DATA_REFLECT(MaterialParamEntry);

		HFIELD();
		std::string m_name;

		HFIELD();
		Engine::MaterialParamType m_type = Engine::MaterialParamType::Float;

		HFIELD();
		Math::Vec4f m_value;

		HFIELD(AssetRef[Reflect::TypeOf<Engine::TextureAsset>()]);
		Engine::AssetHandle<Engine::TextureAsset> m_texture;
	};
}