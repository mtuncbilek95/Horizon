#include "RenderSubmission.h"

#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/Forward+/Resources.h>

namespace Horizon::World::System
{
	b8 RenderSubmission::OnInitialize()
	{
		m_presentation = GetEngine()->RequestService<Engine::PresentationService>();
		if (!m_presentation)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PresentationService is missing, scene has no target to render into");
			return false;
		}

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
			.colorImageDimension = m_presentation->GetSceneExtent(),
			.shadowMapDimension = { 2048, 2048 },
		};

		m_graph->ImportTexture<Rendering::Resource::SceneColor>(m_presentation->GetSceneTexture(), m_presentation->GetSceneView(),
			RHI::ResourceState::Undefined, RHI::ResourceState::ShaderResource);

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