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
		m_graph->OnExecute();
	}

	void RenderSubmission::OnFinalize()
	{
	}

	void RenderSubmission::DeclareDependencies(Engine::SystemDependency& graph)
	{
		//graph.Reads<Component::CameraWorld>();
	}
}