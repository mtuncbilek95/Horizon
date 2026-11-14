#pragma once

#include <Engine/World/System.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class PhysicsService;
}

namespace Horizon::World::System
{
	HCLASS();
	class PhysicsWriteBack : public Engine::System
	{
		HORIZON_TYPE_REFLECT(PhysicsWriteBack);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

	private:
		Engine::PhysicsService* m_physics = nullptr;
	};
}