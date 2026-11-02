#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Input/InputActionMapAsset.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["InputReceiver", "Input"]);
	class InputReceiver final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(InputReceiver);
	public:
		InputReceiver() = default;
		~InputReceiver() = default;

		HFIELD(AssetRef[Reflect::TypeOf<Engine::InputActionMapAsset>()]);
		Engine::AssetHandle<Engine::InputActionMapAsset> m_mapHandle;

		HFIELD();
		u32 m_playerIndex = 0;
	};
}