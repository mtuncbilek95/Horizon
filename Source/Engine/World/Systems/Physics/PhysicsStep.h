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
	class PhysicsStep : public Engine::System
	{
		HORIZON_TYPE_REFLECT(PhysicsStep);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

	private:
		Engine::PhysicsService* m_physics = nullptr;
		f32 m_accumulator = 0.f;
	};
}