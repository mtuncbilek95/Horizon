#pragma once

#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[3000]);
	class H_EXPORT CameraSystem : public System
	{
		HORIZON_TYPE_REFLECT(CameraSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::General; }
	};
}