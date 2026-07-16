#include "AssetSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>
#include <Engine/Asset/IAsset.h>
#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/IAssetLoadStrategy.h>
#include <Engine/Asset/AssetLoadStrategyAttribute.h>

namespace Horizon
{
	EngineReport AssetSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		struct Candidates
		{
			TypeManifest* defaultManifest = nullptr;
			std::vector<TypeManifest*> overrides;
		};

		std::unordered_map<std::string, Candidates> candidates;

		ModuleContext* modCtx = pEngine->GetModuleContext();
		for (TypeManifest* manifest : modCtx->GetManifestsByAttribute(TypeIdOf<AssetLoadStrategyAttribute>()))
		{
			AssetLoadStrategyAttribute* attr = manifest->GetCustomAttribute<AssetLoadStrategyAttribute>();
			if (!attr)
				continue;

			Candidates& c = candidates[std::string(attr->GetAssetType())];

			if (attr->IsDefault())
			{
				if (c.defaultManifest)
					Terminal::Warn("AssetSystem", "Multiple defaults for '{}' — keeping the first", attr->GetAssetType());
				else
					c.defaultManifest = manifest;
			}
			else
			{
				c.overrides.push_back(manifest);
			}
		}

		for (auto& [assetType, c] : candidates)
		{
			TypeManifest* winner = nullptr;

			if (c.overrides.size() == 1)
			{
				winner = c.overrides[0];
				Terminal::Log("AssetSystem", "'{}' strategy overridden by {}", assetType, winner->GetName());
			}
			else if (c.overrides.size() > 1)
			{
				Terminal::Error("AssetSystem", "{} strategies registered for '{}' — ambiguous, reverting to default",
					c.overrides.size(), assetType);
				winner = c.defaultManifest;
			}
			else
			{
				winner = c.defaultManifest;
			}

			if (!winner)
			{
				Terminal::Warn("AssetSystem", "No load strategy for '{}' — assets of this type cannot load", assetType);
				continue;
			}

			m_strategies[assetType] = static_cast<IAssetLoadStrategy*>(winner->Create());
		}

		return EngineReport();
	}

	void AssetSystem::OnSync()
	{
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
		// TODO: PAL::File needs ReadFileRange a.k.a ReadBytes(offset);
		std::vector<u8> headerBytes;

		AssetHeader header;
		if (!DeserializeHeader(headerBytes.data(), headerBytes.size(), header))
		{
			Terminal::Warn("AssetSystem", "Invalid cooked header: {}", cookedPath.string());
			return;
		}

		if (header.guid != guid)
		{
			Terminal::Warn("AssetSystem", "Guid mismatch in {}", cookedPath.string());
			return;
		}

		AssetEntry entry;
		entry.m_guid = header.guid;
		entry.m_type = header.type;
		entry.m_cookedPath = cookedPath;
		entry.m_dependencies = header.dependencies;
		entry.m_settingsOffset = header.settingsOffset;
		entry.m_settingsSize = header.settingsSize;
		entry.m_payloadOffset = header.payloadOffset;
		entry.m_payloadSize = header.payloadSize;
		entry.m_state = AssetState::Registered;

		m_entries[guid] = std::move(entry);
	}

	void AssetSystem::UnregisterAsset(const Guid& guid)
	{
		auto it = m_entries.find(guid);
		if (it == m_entries.end())
			return;

		if (it->second.m_runtime)
			Allocator::Delete(it->second.m_runtime);

		m_entries.erase(it);
	}

	IAsset* AssetSystem::Load(const Guid& guid)
	{
		auto it = m_entries.find(guid);
		if (it == m_entries.end())
		{
			Terminal::Warn("AssetSystem", "Unknown asset");
			return nullptr;
		}

		AssetEntry& entry = it->second;

		if (entry.m_state == AssetState::Loaded || entry.m_state == AssetState::Loading)
			return entry.m_runtime;

		entry.m_state = AssetState::Loading;

		for (const Guid& dep : entry.m_dependencies)
			Load(dep);

		IAssetLoadStrategy* strategy = FindStrategy(entry.m_type);
		if (!strategy)
		{
			Terminal::Warn("AssetSystem", "No load strategy for type '{}'", entry.m_type);
			entry.m_state = AssetState::Failed;
			return nullptr;
		}

		IAsset* asset = strategy->Load(entry);
		if (!asset)
		{
			entry.m_state = AssetState::Failed;
			return nullptr;
		}

		asset->SetGuid(guid);
		entry.m_runtime = asset;
		entry.m_state = AssetState::Loaded;

		return asset;
	}

	void AssetSystem::Unload(const Guid& guid)
	{
		auto it = m_entries.find(guid);
		if (it == m_entries.end() || !it->second.m_runtime)
			return;

		Allocator::Delete(it->second.m_runtime);
		it->second.m_runtime = nullptr;
		it->second.m_state = AssetState::Registered;
	}

	IAssetLoadStrategy* AssetSystem::FindStrategy(std::string_view assetType)
	{
		auto it = m_strategies.find(std::string(assetType));
		return it == m_strategies.end() ? nullptr : it->second;
	}

}