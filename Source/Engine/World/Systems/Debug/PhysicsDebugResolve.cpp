#include "PhysicsDebugResolve.h"

#include <Engine/Physics/PhysicsService.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderablePhysicsDebug.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Systems/Rendering/RenderExtraction.h>
#include <Engine/World/Systems/Rendering/RenderSubmission.h>
#include <Engine/World/Systems/Physics/PhysicsWriteBack.h>

namespace Horizon::World::System
{
	b8 PhysicsDebugResolve::OnInitialize()
	{
		m_physics = GetEngine()->RequestService<Engine::PhysicsService>();
		if (!m_physics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PhysicsService is missing, physics debug lines cannot be collected");
			return false;
		}

		return true;
	}

	void PhysicsDebugResolve::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		auto* pWorld = GetEngine()->RequestService<Engine::WorldService>();
		Engine::Blackboard& board = pWorld->GetBlackboard();

		const Engine::RenderableCamera* pCamera = board.TryGet<Engine::RenderableCamera>();
		if (!pCamera)
			return;

		Engine::FramePhysicsDebug& debug = board.Add<Engine::FramePhysicsDebug>();
		m_physics->CollectDebugLines(pCamera->camWorldPosition, debug.lines);
	}

	void PhysicsDebugResolve::OnFinalize()
	{
	}

	void PhysicsDebugResolve::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Engine::RenderableCamera>();
		graph.Writes<Engine::FramePhysicsDebug>();

		graph.RunsAfter<RenderExtraction, PhysicsWriteBack>();
		graph.RunsBefore<RenderSubmission>();
	}
}