#pragma once

#include <Engine/Core/System.h>
#include <Engine/Asset/AssetTypeRegistry.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace Horizon::Engine
{
	class Asset;
	class AssetLifetimeStrategy;

	class H_EXPORT AssetSystem : public System
	{
	public:
		AppReport OnAttach(Application* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		const AssetTypeRegistry& GetTypeRegistry() const { return m_typeRegistry; }

	private:
		AssetTypeRegistry m_typeRegistry;
	};
}