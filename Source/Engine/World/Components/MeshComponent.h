#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Guid.h>

namespace Horizon::Engine
{
	HCLASS(ComponentType["Mesh Component", "Rendering", true]);
	class H_EXPORT MeshComponent : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(MeshComponent);
	public:
		MeshComponent() = default;
		~MeshComponent() = default;

		HFIELD();
		Guid m_meshId;

		HFIELD();
		List<Guid> m_materialOverrides;
	};
}