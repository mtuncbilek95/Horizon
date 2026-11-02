#include "RenderSubmission.h"

namespace Horizon::World::System
{
	b8 RenderSubmission::OnInitialize()
	{
		return true;
	}

	void RenderSubmission::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
	}

	void RenderSubmission::OnFinalize()
	{
	}

	void RenderSubmission::DeclareDependencies(Engine::SystemDependency& graph)
	{
		//graph.Reads<Component::CameraWorld>();
	}
}