#pragma once

#include <Engine/Asset/IAssetLoadStrategy.h>

namespace Horizon
{
	class DefaultWorldLoadStrategy : public IAssetLoadStrategy 
	{ 
	public:
		IAsset* Load(const AssetEntry& entry) final; 
	};
}