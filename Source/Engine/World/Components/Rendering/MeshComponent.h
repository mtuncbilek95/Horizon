#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/PermittedMeshData.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["MeshComponent", "Rendering"]);
	class ENGINE_API MeshComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(MeshComponent);
	public:
		MeshComponent() = default;
		~MeshComponent() = default;

		HFIELD(AssetRef[Reflect::TypeOf<MeshAsset>()]);
		AssetHandle<MeshAsset> m_meshHandle;

		HFIELD();
		b8 m_hideInRender = false;

		Guid m_resolvedId;
	};
}