#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Engine/Asset/Script/ScriptFieldOverride.h>
#include <Engine/World/Components/Scripts/ScriptLiveValue.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Attributes/HideInInspectorAttribute.h>

#include <string>

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

		HFIELD(HideInInspector);
		List<ScriptFieldOverride> m_overrides;

		HFIELD(HideInInspector);
		List<std::string> m_entityNames;

		HFIELD(HideInInspector);
		List<EntityHandle> m_entityRefs;

		List<ScriptLiveValue> m_liveValues;
	};
}