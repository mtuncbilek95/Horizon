#pragma once

#include <Engine/Asset/IAssetLoadStrategy.h>
#include <Engine/Asset/AssetLoadStrategyAttribute.h>
#include "DefaultWorldLoadStrategy.reflected.h"

namespace Horizon
{
	HCLASS(AssetLoadStrategyAttribute["World", true]);
	class DefaultWorldLoadStrategy : public IAssetLoadStrategy 
	{ 
		HORIZON_REFLECT;
	public:
		IAsset* Load(const AssetEntry& entry) final; 
	};
}