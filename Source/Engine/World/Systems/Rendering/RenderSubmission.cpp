#include "RenderSubmission.h"

#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Location/WorldTransform.h>
#include <Engine/World/Components/Rendering/MeshSelection.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>
#include <Engine/Asset/AssetService.h>
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
		for (Engine::RenderableMesh& renderable : m_drawList.meshes)
			renderable.mesh.pAsset->EndUse(renderable.mesh);
		m_drawList.meshes.Clear();

		currentScene.ForEach<Component::CameraWorld>([&](Engine::EntityHandle, Component::CameraWorld& cam)
			{
				m_drawList.camViewMatrix = cam.m_view;
				m_drawList.camProjMatrix = cam.m_projection;
				m_drawList.camViewProjMatrix = cam.m_viewProjection;
				m_drawList.camWorldPosition = cam.m_worldPosition;
			});

		currentScene.ForEach<Component::WorldTransform, Component::MeshSelection>(
			[&](Engine::EntityHandle, Component::WorldTransform& world, Component::MeshSelection& selection)
			{
				if (selection.m_meshHandle.GetAsset() == nullptr)
					selection.m_meshHandle.SetAsset(m_assets->FindAsset<Engine::MeshAsset>(selection.m_meshHandle.GetId()));

				Engine::MeshAsset* pMesh = selection.m_meshHandle.GetAsset();
				if (!pMesh || (!selection.m_visible && !selection.m_castShadow))
					return;

				if (pMesh->GetResidency() == Engine::AssetResidency::Unloaded)
				{
					pMesh->LoadAsync();
					return;
				}

				Engine::MeshAssetUsePermit permit = pMesh->BeginUse();
				if (!permit.IsValid())
					return;

				m_drawList.meshes.PushBack({ world.m_worldMatrix, permit, selection.m_visible, selection.m_castShadow });
			});

		Engine::RenderSetupContext setupCtx =
		{
			.colorImageDimension = m_presentation->GetSceneExtent(),
			.shadowMapDimension = { 2048, 2048 },
		};

		m_graph->ImportTexture<Rendering::Resource::SceneColor>(m_presentation->GetSceneTexture(), m_presentation->GetSceneView(),
			RHI::ResourceState::Undefined, RHI::ResourceState::ShaderResource);

		m_graph->OnExecute(setupCtx, m_drawList);
	}

	void RenderSubmission::OnFinalize()
	{
	}

	void RenderSubmission::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::CameraWorld>();
		graph.Reads<Component::WorldTransform>();
		graph.Reads<Component::MeshSelection>();

		graph.RunsAfter<LocalToWorld>();
	}
}