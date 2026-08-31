#pragma once

#include <Engine/Asset/Scene/SceneAsset.h>
#include <Engine/World/World.h>

#include <Runtime/Serialization/Archive.h>

namespace Horizon::Engine
{
	class ReflectionSystem;

	class H_EXPORT SceneInstantiator final
	{
	public:
		static b8 Apply(const SceneAsset& scene, World& world, ReflectionSystem* pReflection);

		static b8 Capture(World& world, ReflectionSystem* pReflection, IArchiveWriter& writer);
		static void CaptureEmpty(IArchiveWriter& writer);
	};
}