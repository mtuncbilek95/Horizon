#include "AssetStreamer.h"

namespace Horizon::Engine
{
	void AssetStreamer::FailedAssetLog(AssetObject* pAsset, std::string_view reason)
	{
		Terminal::Error(StringOps::GetName(this), "{} Path: {}", reason, pAsset->m_ownerEntry.assetId.ToString());
		pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
	}
}