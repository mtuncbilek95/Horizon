#include "MeshSystem.h"

#include <Engine/Asset/Mesh/MeshAsset.h>

namespace Horizon::Engine
{
	b8 MeshSystem::OnInitialize()
	{
		m_assetService = GetEngine()->RequestService<AssetService>();

		return m_assetService != nullptr;
	}

	void MeshSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		currentScene.ForEach<MeshComponent>([&](EntityHandle handl, MeshComponent& mesh)
			{
				const Guid& id = mesh.m_meshHandle.GetId();

				if (!id.IsValid())
					return;

				MeshAsset* pAsset = mesh.m_meshHandle.GetAsset();

				if (pAsset != nullptr && pAsset->GetPhysicalEntry().assetId == id)
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

				mesh.m_meshHandle.SetAsset(m_assetService->FindAsset<MeshAsset>(id));
				mesh.m_resolvedId = id;
			});
	}

	void MeshSystem::OnFinalize()
	{
	}

	void MeshSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}
}