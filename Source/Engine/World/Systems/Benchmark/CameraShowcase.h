#pragma once

#include <Engine/World/System.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::System
{
	HCLASS();
	class CameraShowcase : public Engine::System
	{
		HORIZON_TYPE_REFLECT(CameraShowcase);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;
	};
}