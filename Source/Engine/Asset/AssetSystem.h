#pragma once

#include <Engine/Core/System.h>
#include <Engine/Asset/AssetEntry.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Horizon
{
	class IAsset;
	class IAssetLoadStrategy;

	class H_EXPORT AssetSystem : public System
	{
	public:
		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		void RegisterAsset(const Guid& guid, const std::filesystem::path& cookedPath);
		void UnregisterAsset(const Guid& guid);

		IAsset* Load(const Guid& guid);
		void Unload(const Guid& guid);

		template<typename T>
		T* LoadAs(const Guid& guid) { return static_cast<T*>(Load(guid)); }

	private:
		IAssetLoadStrategy* FindStrategy(std::string_view assetType);
		b8 ReadDependencies(const AssetEntry& entry, std::vector<Guid>& out);

	private:
		std::unordered_map<Guid, AssetEntry> m_entries;
		std::unordered_map<std::string, IAssetLoadStrategy*> m_strategies;
	};
}