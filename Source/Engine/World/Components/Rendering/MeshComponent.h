#pragma once

#include <Engine/Asset/AssetHandle.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/Rendering/MeshDrawRange.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["MeshComponent", "Rendering"]);
	class H_EXPORT MeshComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(MeshComponent);
	public:
		MeshComponent() = default;
		~MeshComponent() = default;

		HFIELD();
		AssetHandle<MeshAsset> m_meshHandle;

		List<MeshDrawRange> m_drawRanges;
		Guid m_resolvedId;
		b8 m_resident = false;
	};
}