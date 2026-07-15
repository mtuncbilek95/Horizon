#pragma once

#include <Engine/Core/System.h>

#include <unordered_map>

namespace Horizon
{
	class IAsset;

	class H_EXPORT AssetSystem : public System
	{
	public:
		AssetSystem() = default;
		~AssetSystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		std::unordered_map<Guid, IAsset*> m_registeredAssets;
	};
}