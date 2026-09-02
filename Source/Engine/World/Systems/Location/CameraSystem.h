#pragma once

#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS(SystemOrder[200]);
	class H_EXPORT CameraSystem : public System
	{
		HORIZON_TYPE_REFLECT(CameraSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;

		void ResizeViewport(const Math::Vec2u& viewport) { m_workableArea = viewport; }
		
	private:
		Math::Vec2u m_workableArea;
	};
}