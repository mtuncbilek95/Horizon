#include "PhysicsStep.h"

#include <Engine/Physics/PhysicsService.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/Systems/Physics/PhysicsBodySync.h>
#include <Runtime/Containers/StringHelper.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <algorithm>

namespace Horizon::World::System
{
	b8 PhysicsStep::OnInitialize()
	{
		m_physics = GetEngine()->RequestService<Engine::PhysicsService>();
		if (!m_physics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsService is missing, world cannot be stepped");
			return false;
		}

		return true;
	}

	void PhysicsStep::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		JPH::PhysicsSystem* pWorld = m_physics->GetWorld();

		m_accumulator += ctx.DeltaTime();

		u32 steps = 0;
		while (m_accumulator >= Engine::PhysicsService::FixedStep && steps < Engine::PhysicsService::MaxStepsPerFrame)
		{
			const JPH::EPhysicsUpdateError error = pWorld->Update(Engine::PhysicsService::FixedStep, 1, m_physics->GetTempAllocator(), m_physics->GetJobBridge());
			if (error != JPH::EPhysicsUpdateError::None)
				Terminal::Warn(StringHelper::GetTypeName(this), "Jolt step reported error flags {}", static_cast<u32>(error));

			m_accumulator -= Engine::PhysicsService::FixedStep;
			steps++;
		}

		if (steps == Engine::PhysicsService::MaxStepsPerFrame)
			m_accumulator = std::min(m_accumulator, Engine::PhysicsService::FixedStep);
	}

	void PhysicsStep::OnFinalize()
	{
	}

	void PhysicsStep::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.RunsAfter<PhysicsBodySync>();
	}
}