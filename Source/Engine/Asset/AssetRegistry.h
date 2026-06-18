#pragma once

#include <Runtime/Containers/Guid.h>
#include <Engine/Asset/AssetMetadata.h>

#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <string>

namespace Horizon
{
	class AssetRegistry
	{
	public:
		std::optional<AssetMetadata> Find(Guid guid) const 
		{
			std::shared_lock lock(m_mutex);
			auto it = m_entries.find(guid);
			if (it == m_entries.end()) return std::nullopt;
			return it->second;
		}

		b8 Contains(Guid guid) const
		{
			std::shared_lock lock(m_mutex);
			return m_entries.find(guid) != m_entries.end();
		}

		void Add(const AssetMetadata& meta)
		{
			std::unique_lock lock(m_mutex);
			m_entries[meta.guid] = meta;
		}

		void Remove(Guid guid)
		{
			std::unique_lock lock(m_mutex);
			m_entries.erase(guid);
		}

		void Clear()
		{
			std::unique_lock lock(m_mutex);
			m_entries.clear();
		}

		b8 LoadFromFile(const std::string& path);
		b8 SaveToFile(const std::string& path) const;

	private:
		mutable std::shared_mutex m_mutex;
		std::unordered_map<Guid, AssetMetadata, GuidHash> m_entries;
	};
}