#include "RenderSubmission.h"

namespace Horizon::World::System
{
	b8 RenderSubmission::OnInitialize()
	{
		m_graph = MakeUnique<Engine::RenderGraph>(GetEngine());
		
		if (!m_graph->OnInitialize())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "RenderGraph has failed to initialize");
			return false;
		}

		return true;
	}

	void RenderSubmission::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		Engine::RenderSetupContext setupCtx = 
		{
			.colorImageDimension = { 1920, 1080 },
			.shadowMapDimension = { 2048, 2048 },
		};

		Engine::RenderDrawContext drawCtx = {};
		m_graph->OnExecute(setupCtx, drawCtx);
	}

	void RenderSubmission::OnFinalize()
	{
	}

	void RenderSubmission::DeclareDependencies(Engine::SystemDependency& graph)
	{
		//graph.Reads<Component::CameraWorld>();
	}
}