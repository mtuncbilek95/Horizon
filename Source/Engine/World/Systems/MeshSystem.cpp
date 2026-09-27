#include "MeshSystem.h"

#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Engine/World/Components/MeshComponent.h>

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
				{
					ClearResidency(mesh);
					return;
				}

				MeshAsset* pAsset = mesh.m_meshHandle.GetAsset();

				if (pAsset == nullptr || pAsset->GetPhysicalEntry().assetId != id)
				{
					if (!m_assetService->HasAsset(id))
					{
						if (mesh.m_resolvedId != id)
						{
							Terminal::Warn(StringOps::GetName(this), "{} is not registered yet, mesh stays hidden until it arrives", id.ToString());
							mesh.m_resolvedId = id;
						}

						ClearResidency(mesh);
						return;
					}

					pAsset = m_assetService->FindAsset<MeshAsset>(id);
					mesh.m_meshHandle.SetAsset(pAsset);
				}

				const AssetResidency state = pAsset->GetResidencyState();

				if (state == AssetResidency::Unloaded)
					pAsset->LoadAsync();

				if (state != AssetResidency::Resident)
				{
					ClearResidency(mesh);
					return;
				}

				if (!mesh.m_resident || mesh.m_resolvedId != id)
				{
					BuildDrawRanges(mesh, pAsset);
					mesh.m_resolvedId = id;
					mesh.m_resident = true;
				}
			});
	}

	void MeshSystem::OnFinalize()
	{
	}

	void MeshSystem::ClearResidency(MeshComponent& mesh)
	{
		if (!mesh.m_resident)
			return;

		mesh.m_drawRanges.Clear();
		mesh.m_resolvedId = Guid();
		mesh.m_resident = false;
	}

	void MeshSystem::BuildDrawRanges(MeshComponent& mesh, const MeshAsset* pAsset)
	{
		const List<MeshSubMesh>& subMeshes = pAsset->GetSubmeshes();
		const u32 firstVertex = pAsset->GetFirstVertex();
		const u32 firstIndex = pAsset->GetFirstIndex();

		mesh.m_drawRanges.Clear();
		mesh.m_drawRanges.Reserve(subMeshes.GetCount());

		for (const MeshSubMesh& subMesh : subMeshes)
		{
			MeshDrawRange range = {};
			range.firstVertex = firstVertex + subMesh.vertexOffset;
			range.firstIndex = firstIndex + subMesh.indexOffset;
			range.indexCount = subMesh.indexCount;

			mesh.m_drawRanges.PushBack(range);
		}
	}
}