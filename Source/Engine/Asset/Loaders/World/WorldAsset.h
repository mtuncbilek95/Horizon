#pragma once

#include <Engine/Asset/IAsset.h>
#include <Engine/ECS/WorldRegistry.h>

#include <string>
#include <unordered_map>

namespace Horizon
{
	class WorldAsset : public IAsset
	{
	public:
		WorldRegistry& GetRegistry() { return m_registry; }
		const WorldRegistry& GetRegistry() const { return m_registry; }

		const std::string& GetWorldName() const { return m_worldName; }
		void SetWorldName(std::string name) { m_worldName = std::move(name); }

		const std::string* NameOf(EntityHandle entity) const
		{
			auto it = m_entityNames.find(entity.id);
			return it == m_entityNames.end() ? nullptr : &it->second;
		}

		void SetName(EntityHandle entity, std::string name) { m_entityNames[entity.id] = std::move(name); }

	private:
		std::string m_worldName;
		WorldRegistry m_registry;
		std::unordered_map<u32, std::string> m_entityNames;
	};
}