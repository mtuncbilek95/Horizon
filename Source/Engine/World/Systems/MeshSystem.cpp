#include "MeshSystem.h"

#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Engine/World/Components/MeshComponent.h>

namespace Horizon::Engine
{
	b8 MeshSystem::OnInitialize()
	{
		return true;
	}

	void MeshSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		currentScene.ForEach<MeshComponent>([&](EntityHandle handl, MeshComponent& mesh)
			{
				const AssetHandle<MeshAsset>& meshHandle = mesh.m_meshHandle;

				if (!meshHandle.GetId().IsValid())
				{
					if (mesh.m_resident)
					{
						mesh.m_drawRanges.Clear();
						mesh.m_resolvedId = Guid();
						mesh.m_resident = false;
					}

					return;
				}

				MeshAsset* pAsset = meshHandle.GetAsset();

				if (pAsset == nullptr)
				{
					if (mesh.m_resident)
					{
						mesh.m_drawRanges.Clear();
						mesh.m_resolvedId = Guid();
						mesh.m_resident = false;
					}

					return;
				}

				const AssetResidency state = pAsset->GetResidencyState();

				if (state == AssetResidency::Unloaded)
					pAsset->LoadAsync();

				if (state != AssetResidency::Resident)
				{
					if (mesh.m_resident)
					{
						mesh.m_drawRanges.Clear();
						mesh.m_resolvedId = Guid();
						mesh.m_resident = false;
					}

					return;
				}

				if (!mesh.m_resident || mesh.m_resolvedId != meshHandle.GetId())
				{
					const List<MeshSubMesh>& subMeshes = pAsset->GetSubmeshes();
					const u32 firstVertex = pAsset->GetFirstVertex();
					const u32 firstIndex = pAsset->GetFirstIndex();

					mesh.m_drawRanges.Clear();
					mesh.m_drawRanges.Reserve(subMeshes.GetCount());

					for (usize i = 0; i < subMeshes.GetCount(); ++i)
					{
						const MeshSubMesh& subMesh = subMeshes[i];

						MeshDrawRange range = {};
						range.firstVertex = firstVertex + subMesh.vertexOffset;
						range.firstIndex = firstIndex + subMesh.indexOffset;
						range.indexCount = subMesh.indexCount;

						mesh.m_drawRanges.PushBack(range);
					}

					mesh.m_resolvedId = mesh.m_meshHandle.GetId();
					mesh.m_resident = true;
				}
			});
	}

	void MeshSystem::OnFinalize()
	{
	}
}