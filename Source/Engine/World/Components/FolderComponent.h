#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(ComponentId["", ""]);
	class FolderComponent final : public ComponentObject
	{
		HORIZON_TYPE_REFLECT(FolderComponent);
	public:
		FolderComponent() = default;
		~FolderComponent() = default;

		HFIELD();
		std::string m_path;

		HFIELD();
		u32 m_order;
	};
}