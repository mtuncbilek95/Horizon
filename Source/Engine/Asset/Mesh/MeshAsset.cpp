#include "MeshAsset.h"

#include <Engine/Asset/Mesh/MeshAssetStreamer.h>

namespace Horizon::Engine
{
	void MeshAsset::BeginUse()
	{
		GetStreamer<MeshAssetStreamer>()->BeginUse(this);
	}

	void MeshAsset::EndUse()
	{
		GetStreamer<MeshAssetStreamer>()->EndUse(this);
	}
}