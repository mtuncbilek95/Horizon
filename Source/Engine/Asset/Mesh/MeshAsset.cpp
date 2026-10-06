#include "MeshAsset.h"

#include <Engine/Asset/Mesh/MeshAssetStreamer.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	void MeshAsset::LoadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		m_streamer->LoadAsync(this);
	}

	PermittedMeshData MeshAsset::BeginUse()
	{
		return GetStreamer<MeshAssetStreamer>()->BeginUse(this);
	}

	void MeshAsset::EndUse(const PermittedMeshData& permit)
	{
		if (permit.pAsset != this)
		{
			Terminal::Error(StringOps::GetName(this), "Permit belongs to another mesh, use count left untouched");
			return;
		}

		GetStreamer<MeshAssetStreamer>()->EndUse(permit);
	}
}