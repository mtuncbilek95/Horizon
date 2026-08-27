#pragma once

#include <Engine/Asset/Scene/SceneAsset.h>
#include <Engine/World/World.h>

namespace Horizon::Engine
{
	class ReflectionSystem;

	class H_EXPORT SceneInstantiator final
	{
	public:
		static b8 Apply(const SceneAsset& scene, World& world, ReflectionSystem* pReflection);
	};
}