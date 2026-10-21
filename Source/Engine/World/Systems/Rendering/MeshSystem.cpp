#include "MeshSystem.h"

#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Graphics/GraphicsContext.h>

namespace Horizon::Engine
{
	b8 MeshSystem::OnInitialize()
	{
		m_assetService = GetEngine()->RequestService<AssetService>();
		if (m_assetService == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "AssetService is unavailable, meshes cannot be resolved");
			return false;
		}

		GraphicsContext* pGraphics = GetEngine()->RequestContext<GraphicsContext>();
		if (pGraphics == nullptr || pGraphics->GetDevice() == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, meshes cannot be unloaded safely");
			return false;
		}

		m_device = pGraphics->GetDevice();
		return true;
	}

	void MeshSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		if (m_lastScene != &currentScene)
		{
			UnloadSceneMeshes();
			m_lastScene = &currentScene;
		}

		currentScene.ForEach<MeshComponent>([&](EntityHandle handl, MeshComponent& mesh)
			{
				const Guid& id = mesh.m_meshHandle.GetId();

				mesh.m_meshHandle.SetAsset(nullptr);

				if (!id.IsValid())
					return;

				if (!m_assetService->HasAsset(id))
				{
					if (mesh.m_resolvedId != id)
					{
						Terminal::Warn(StringOps::GetName(this), "{} is not registered yet, mesh stays hidden until it arrives", id.ToString());
						mesh.m_resolvedId = id;
					}

					return;
				}

				MeshAsset* pAsset = m_assetService->FindAsset<MeshAsset>(id);
				mesh.m_resolvedId = id;

				if (!pAsset)
					return;

				mesh.m_meshHandle.SetAsset(pAsset);
				pAsset->LoadAsync();
				m_loadedMeshes.insert(id);
			});
	}

	void MeshSystem::OnFinalize()
	{
	}

	void MeshSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	void MeshSystem::UnloadSceneMeshes()
	{
		if (m_loadedMeshes.empty())
			return;

		m_device->WaitIdle();

		for (const Guid& id : m_loadedMeshes)
		{
			if (!m_assetService->HasAsset(id))
				continue;

			MeshAsset* pAsset = m_assetService->FindAsset<MeshAsset>(id);
			if (pAsset)
				pAsset->UnloadAsync();
		}

		Terminal::Info(StringOps::GetName(this), "{} meshes of the previous scene have been unloaded", m_loadedMeshes.size());
		m_loadedMeshes.clear();
	}
}