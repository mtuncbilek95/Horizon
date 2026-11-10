#include "RenderExtraction.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Rendering/RenderableCamera.h>
#include <Engine/Rendering/RenderableMesh.h>
#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Location/WorldTransform.h>
#include <Engine/World/Components/Rendering/MeshSelection.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>

namespace Horizon::World::System
{
	b8 RenderExtraction::OnInitialize()
	{
		m_assets = GetEngine()->RequestService<Engine::AssetService>();
		if (!m_assets)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, scene has no asset setter to render");
			return false;
		}

		return true;
	}

	void RenderExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		for (Asset::MeshUsePermit& permit : m_permits)
			permit.pAsset->EndUse(permit);

		m_permits.Clear();

		auto* pWorld = GetEngine()->RequestService<Engine::WorldService>();
		Engine::Blackboard& board = pWorld->GetBlackboard();

		Engine::RenderableCamera& camera = board.Add<Engine::RenderableCamera>();
		Engine::FrameMesh& frame = board.Add<Engine::FrameMesh>();

		// Get camera
		currentScene.ForEach<Component::CameraWorld>([&](Engine::EntityHandle, Component::CameraWorld& cam)
			{
				camera.camViewMatrix = cam.m_view;
				camera.camProjMatrix = cam.m_projection;
				camera.camViewProjMatrix = cam.m_viewProjection;
				camera.camWorldPosition = cam.m_worldPosition;
			});

		// Resolve mesh
		currentScene.ForEach<Component::WorldTransform, Component::MeshSelection>(
			[&](Engine::EntityHandle, Component::WorldTransform& world, Component::MeshSelection& selection)
			{
				if (selection.m_meshHandle.GetAsset() == nullptr)
					selection.m_meshHandle.SetAsset(m_assets->FindAsset<Asset::Mesh>(selection.m_meshHandle.GetId()));

				Asset::Mesh* pMesh = selection.m_meshHandle.GetAsset();
				if (!pMesh || (!selection.m_visible && !selection.m_castShadow))
					return;

				if (pMesh->GetResidency() == Engine::AssetResidency::Unloaded)
				{
					pMesh->LoadAsync();
					return;
				}

				Asset::MeshUsePermit permit = pMesh->BeginUse();
				if (!permit.IsValid())
					return;

				m_permits.PushBack(permit);
				frame.meshes.PushBack({ world.m_worldMatrix, permit, selection.m_visible, selection.m_castShadow });
			});
	}

	void RenderExtraction::OnFinalize()
	{
	}

	void RenderExtraction::DeclareDependencies(Engine::SystemDependency& graph)
	{
		graph.Reads<Component::CameraWorld>();
		graph.Reads<Component::WorldTransform>();
		graph.Reads<Component::MeshSelection>();

		graph.Writes<Engine::RenderableCamera>();
		graph.Writes<Engine::FrameMesh>();

		graph.RunsAfter<LocalToWorld>();
	}
}