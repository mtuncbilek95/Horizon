#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Mesh/Mesh.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["MeshSelection", "Rendering"]);
	class MeshSelection final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(MeshSelection);
	public:
		MeshSelection() = default;
		~MeshSelection() = default;

		HFIELD(AssetRef[Reflect::TypeOf<Asset::Mesh>()]);
		Engine::AssetHandle<Asset::Mesh> m_meshHandle;

		HFIELD();
		b8 m_visible = true;

		HFIELD();
		b8 m_castShadow = true;

	};
}