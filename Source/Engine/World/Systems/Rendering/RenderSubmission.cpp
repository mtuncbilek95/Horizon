#include "RenderSubmission.h"

#include <Engine/World/Components/Benchmark/PointLightBatch.h>
#include <Engine/World/Components/Camera/CameraWorld.h>
#include <Engine/World/Components/Location/WorldTransform.h>
#include <Engine/World/Components/Location/LocalTransform.h>
#include <Engine/World/Components/Rendering/DirectionalLight.h>
#include <Engine/World/Components/Rendering/PointLight.h>
#include <Engine/World/Components/Rendering/SpotLight.h>
#include <Engine/World/Components/Rendering/AreaLight.h>
#include <Engine/World/Components/Rendering/MeshSelection.h>
#include <Engine/World/Systems/Location/LocalToWorld.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Forward+/ForwardPass.h>

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
		m_drawList.directionalLights.Clear();
		m_drawList.pointLights.Clear();
		m_drawList.spotLights.Clear();
		m_drawList.areaLights.Clear();

		// Resolve camera
		currentScene.ForEach<Component::CameraWorld>([&](Engine::EntityHandle, Component::CameraWorld& cam)
			{
				m_drawList.camViewMatrix = cam.m_view;
				m_drawList.camProjMatrix = cam.m_projection;
				m_drawList.camViewProjMatrix = cam.m_viewProjection;
				m_drawList.camWorldPosition = cam.m_worldPosition;
			});

		// Resolve meshes
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

				m_drawList.meshes.PushBack({ world.m_worldMatrix, permit, selection.m_visible, selection.m_castShadow });
			});

		// Resolve sun light
		currentScene.ForEach<Component::DirectionalLight>([&](Engine::EntityHandle, Component::DirectionalLight& light)
			{
				m_drawList.directionalLights.PushBack({ light.m_color, light.m_direction, light.m_intensity });
			});

		// Resolve point lights
		currentScene.ForEach<Component::LocalTransform, Component::PointLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::PointLight& light)
			{
				m_drawList.pointLights.PushBack({ light.m_color, local.m_position, light.m_intensity, light.m_range });
			});

		currentScene.ForEach<Component::PointLightBatch>([&](Engine::EntityHandle, Component::PointLightBatch& batch)
			{
				for (const Engine::RenderablePointLight& light : batch.m_lights)
					m_drawList.pointLights.PushBack(light);
			});


		// Resolve spot lights
		currentScene.ForEach<Component::LocalTransform, Component::SpotLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::SpotLight& light)
			{
				const Math::Quat rotation = Math::Quat::FromEuler(local.m_rotation);

				m_drawList.spotLights.PushBack({ light.m_color, local.m_position,
					rotation.RotateVector(Math::Vec3f(0.f, 0.f, -1.f)),
					light.m_intensity, light.m_range, light.m_innerConeAngle, light.m_outerConeAngle });
			});

		// Resolve area lights
		currentScene.ForEach<Component::LocalTransform, Component::AreaLight>(
			[&](Engine::EntityHandle, Component::LocalTransform& local, Component::AreaLight& light)
			{
				const Math::Quat rotation = Math::Quat::FromEuler(local.m_rotation);

				m_drawList.areaLights.PushBack({ light.m_color, local.m_position,
					rotation.RotateVector(Math::Vec3f(1.f, 0.f, 0.f)),
					rotation.RotateVector(Math::Vec3f(0.f, 1.f, 0.f)),
					light.m_intensity, light.m_width, light.m_height, light.m_twoSided });
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
		graph.Reads<Component::LocalTransform>();
		graph.Reads<Component::MeshSelection>();
		graph.Reads<Component::DirectionalLight>();
		graph.Reads<Component::PointLight>();
		graph.Reads<Component::PointLightBatch>();
		graph.Reads<Component::SpotLight>();
		graph.Reads<Component::AreaLight>();

		graph.RunsAfter<LocalToWorld>();
	}
}