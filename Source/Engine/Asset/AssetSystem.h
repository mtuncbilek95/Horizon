#pragma once

#include <Engine/Core/System.h>
#include <Engine/Asset/AssetTypeRegistry.h>
#include <Engine/Asset/AssetEntryDesc.h>
#include <Runtime/Containers/Guid.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class Asset;
	class AssetEntry;

	class H_EXPORT AssetSystem : public System
	{
	public:
		AppReport OnAttach(Application* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		void RegisterAsset(const AssetEntryDesc& desc);
		void UnregisterAsset(const Guid& id);

	private:
		AssetTypeRegistry m_typeRegistry;
		std::unordered_map<Guid, AssetEntry*> m_registeredAssets;
	};
}