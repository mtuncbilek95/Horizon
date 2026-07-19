#pragma once

#include <Engine/Core/System.h>
#include <Engine/Asset/AssetRegistryDesc.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Horizon
{
	class Asset;
	class AssetLifetimeStrategy;

	class H_EXPORT AssetSystem : public System
	{
	public:
		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		void RegisterAsset(const AssetRegistryDesc& registerInfo);
		void UnregisterAsset(const Guid& guid);
	};
}