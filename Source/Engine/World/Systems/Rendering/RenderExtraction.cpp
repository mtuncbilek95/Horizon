#include "RenderExtraction.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Material/MaterialStreamer.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableMesh.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Camera/PerspectiveCamera.h>
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

		m_materials = m_assets->FindLoader<Asset::MaterialStreamer>(Reflect::TypeOf<Asset::Material>());
		if (!m_materials)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "MaterialStreamer is missing, scene has no material buffer to render");
			return false;
		}

		return true;
	}

	void RenderExtraction::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		for (Asset::MeshUsePermit& permit : m_permits)
			permit.pAsset->EndUse(permit);

		m_permits.Clear();

		for (Asset::MaterialUsePermit& permit : m_materialPermits)
			permit.pAsset->EndUse(permit);

		m_materialPermits.Clear();

		auto* pWorld = GetEngine()->RequestService<Engine::WorldService>();
		Engine::Blackboard& board = pWorld->GetBlackboard();

		Engine::RenderableCamera& camera = board.Add<Engine::RenderableCamera>();
		Engine::FrameMesh& frame = board.Add<Engine::FrameMesh>();

		frame.materialBufferView = m_materials->GetMaterialBufferView();
		frame.samplerView = m_materials->GetDefaultSamplerView();

		// Get camera
		currentScene.ForEach<Component::CameraWorld, Component::PerspectiveCamera>([&](Engine::EntityHandle, Component::CameraWorld& cam, Component::PerspectiveCamera& perspective)
			{
				camera.camViewMatrix = cam.m_view;
				camera.camProjMatrix = cam.m_projection;
				camera.camViewProjMatrix = cam.m_viewProjection;
				camera.camWorldPosition = cam.m_worldPosition;
				camera.nearPlane = perspective.m_nearPlane;
				camera.farPlane = perspective.m_farPlane;
			});

		// Resolve mesh
		currentScene.ForEach<Component::WorldTransform, Component::MeshSelection>(
			[&](Engine::EntityHandle, Component::WorldTransform& world, Component::MeshSelection& selection)
			{
				Asset::Mesh* pMesh = m_assets->Resolve(selection.m_meshHandle);
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

				List<Engine::AssetHandle<Asset::Material>>& slots = pMesh->GetMaterialSlots();
				const u32 firstMaterial = static_cast<u32>(frame.materialIndices.GetCount());
				const u32 materialCount = static_cast<u32>(slots.GetCount());

				for (u32 i = 0; i < materialCount; ++i)
					frame.materialIndices.PushBack(ResolveMaterial(slots[i]));

				frame.meshes.PushBack({ world.m_worldMatrix, permit, firstMaterial, materialCount, selection.m_visible, selection.m_castShadow });
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
		graph.Reads<Component::PerspectiveCamera>();

		graph.Writes<Engine::RenderableCamera>();
		graph.Writes<Engine::FrameMesh>();

		graph.RunsAfter<LocalToWorld>();
	}

	u32 RenderExtraction::ResolveMaterial(Engine::AssetHandle<Asset::Material>& handle)
	{
		if (handle.GetId() == Guid())
			return 0;

		Asset::Material* pMaterial = m_assets->Resolve(handle);
		if (!pMaterial)
			return 0;

		if (pMaterial->GetResidency() == Engine::AssetResidency::Unloaded)
		{
			pMaterial->LoadAsync();
			return 0;
		}

		Asset::MaterialUsePermit permit = pMaterial->BeginUse();
		if (!permit.IsValid())
			return 0;

		m_materialPermits.PushBack(permit);
		return permit.materialIndex;
	}
}