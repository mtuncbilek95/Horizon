#include "RenderSubmission.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/RenderableCamera.h>
#include <Engine/Rendering/RenderableMesh.h>
#include <Engine/Rendering/RenderablePointLight.h>
#include <Engine/Rendering/RenderableSpotLight.h>
#include <Engine/Rendering/RenderableAreaLight.h>
#include <Engine/Rendering/RenderableDirectionalLight.h>

#include <Engine/World/WorldService.h>
#include <Engine/World/Systems/Rendering/RenderExtraction.h>
#include <Engine/World/Systems/Rendering/LightExtraction.h>

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

		m_assets = GetEngine()->RequestService<Engine::AssetService>();
		if (!m_assets)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, scene has no asset setter to render");
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

		auto* pWorld = GetEngine()->RequestService<Engine::WorldService>();
		m_graph->OnExecute(setupCtx, pWorld->GetBlackboard());
	}

	void RenderSubmission::OnFinalize()
	{
	}

	void RenderSubmission::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Engine::RenderableCamera>();
		graph.Reads<Engine::FrameMesh>();
		graph.Reads<Engine::RenderableDirectionalLight>();
		graph.Reads<Engine::FramePointLight>();
		graph.Reads<Engine::FrameSpotLight>();
		graph.Reads<Engine::FrameAreaLight>();

		graph.RunsAfter<RenderExtraction, LightExtraction>();
	}
}