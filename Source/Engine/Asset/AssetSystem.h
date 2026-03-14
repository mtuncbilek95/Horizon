#pragma once

#include <Engine/System/System.h>

namespace Horizon
{
	class AssetSystem : public System<AssetSystem>
	{
	public:
		AssetSystem() = default;
		virtual ~AssetSystem() = default;

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;
	};
}