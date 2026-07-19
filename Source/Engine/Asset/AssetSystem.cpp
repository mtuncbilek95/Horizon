#include "AssetSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>
#include <Engine/Asset/IAsset.h>
#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/IAssetLoadStrategy.h>
#include <Engine/Asset/AssetLoadStrategyAttribute.h>

#include <Runtime/PAL/File/File.h>

#include <cstring>

namespace Horizon
{
	EngineReport AssetSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		return EngineReport();
	}

	void AssetSystem::OnSync()
	{
		// TODO: free pending load/stream
	}

	void AssetSystem::OnDetach()
	{
		for (auto& [guid, entry] : m_entries)
		{
			if (entry.m_runtime)
				Allocator::Delete(entry.m_runtime);
		}
		m_entries.clear();

		for (auto& [type, strategy] : m_strategies)
			Allocator::Delete(strategy);

		m_strategies.clear();
	}

	void AssetSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void AssetSystem::GetExecutionOrder(OrderRules& rules) const
	{
	}

	void AssetSystem::RegisterAsset(const Guid& guid, const std::filesystem::path& cookedPath)
	{
	}

	void AssetSystem::UnregisterAsset(const Guid& guid)
	{
	}

	IAsset* AssetSystem::Load(const Guid& guid)
	{
		return nullptr;
	}

	void AssetSystem::Unload(const Guid& guid)
	{
	}

	IAssetLoadStrategy* AssetSystem::FindStrategy(std::string_view assetType)
	{
		auto it = m_strategies.find(std::string(assetType));
		return it == m_strategies.end() ? nullptr : it->second;
	}

	b8 AssetSystem::ReadDependencies(const AssetEntry& entry, std::vector<Guid>& out)
	{
		return true;
	}

}