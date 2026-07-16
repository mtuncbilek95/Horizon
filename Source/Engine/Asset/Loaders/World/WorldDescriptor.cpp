#include "WorldDescriptor.h"

#include <nlohmann/json.hpp>

namespace Horizon
{
	WorldDescriptor::WorldDescriptor(const std::string& worldName, u32 entityCount) : m_worldName(worldName), 
		m_entityCount(entityCount)
	{

	}

	void WorldDescriptor::Serialize(std::vector<u8>& out) const
	{
		nlohmann::json j;
		j["worldName"] = m_worldName;
		j["entityCount"] = m_entityCount;

		std::string s = j.dump();
		out.assign(s.begin(), s.end());
	}

	b8 WorldDescriptor::Deserialize(const u8* data, u64 size)
	{
		auto j = nlohmann::json::parse(std::string((const c8*)data, size), nullptr, false);
		if (j.is_discarded())
			return false;

		m_worldName = j.value("worldName", std::string{});
		m_entityCount = j.value("entityCount", 0u);
		return true;
	}

}