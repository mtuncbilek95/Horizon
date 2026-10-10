#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["ScriptComponent", "Script"]);
	class ENGINE_API ScriptComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(ScriptComponent);
	public:
		ScriptComponent() = default;
		~ScriptComponent() = default;

		HFIELD(AssetRef[Reflect::TypeOf<ScriptAsset>()]);
		AssetHandle<ScriptAsset> m_script;
	};
}