#include "CacheContext.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Horizon
{
	namespace fs = std::filesystem;
	using namespace nlohmann;

	EngineReport CacheContext::OnInitialize()
	{
		const fs::path cachePath = "horizonCache.json";

		if (!fs::exists(cachePath))
		{
			std::ofstream create(cachePath);
			if (!create.is_open())
				return EngineReport("CacheContext", "Failed to create cache file: " + cachePath.string());
			create << "{}";
			create.close();
		}

		std::ifstream file(cachePath);
		if (!file.is_open())
			return EngineReport("CacheContext", "Failed to open cache file: " + cachePath.string());

		json j;
		try
		{
			j = json::parse(file);
		}
		catch (const json::parse_error& e)
		{
			return EngineReport("CacheContext", "JSON parse error: " + std::string(e.what()));
		}

		if (j.contains("Graphics"))
		{
			auto& g = j["Graphics"];
			if (g.contains("appName"))
				m_gfxCache.appName = g["appName"].get<std::string>();
			if (g.contains("engineName"))
				m_gfxCache.engineName = g["engineName"].get<std::string>();
			if (g.contains("appVersion"))
			{
				auto& v = g["appVersion"];
				if (!v.is_array() || v.size() != 3)
					return EngineReport("CacheContext", "Invalid appVersion format, expected array of 3");
				m_gfxCache.appVersion = { v[0].get<u32>(), v[1].get<u32>(), v[2].get<u32>() };
			}
			if (g.contains("imageCount"))
				m_gfxCache.imageCount = g["imageCount"].get<u32>();
			if (g.contains("presentType"))
				m_gfxCache.presentType = static_cast<PresentMode>(g["presentType"].get<int>());
			if (g.contains("rhiType"))
				m_gfxCache.rhiType = static_cast<GfxType>(g["rhiType"].get<int>());
		}

		if (j.contains("Window"))
		{
			auto& w = j["Window"];
			if (w.contains("name"))
				m_windowCache.name = w["name"].get<std::string>();
			if (w.contains("windowSize"))
			{
				auto& s = w["windowSize"];
				if (!s.is_array() || s.size() != 2)
					return EngineReport("CacheContext", "Invalid windowSize format, expected array of 2");
				m_windowCache.windowSize = { s[0].get<u32>(), s[1].get<u32>() };
			}
			if (w.contains("mode"))
				m_windowCache.mode = static_cast<WindowMode>(w["mode"].get<int>());
		}

		return EngineReport();
	}

	void CacheContext::OnFinalize()
	{
		const fs::path cachePath = "horizonCache.json";

		json j;
		j["Graphics"]["appName"] = m_gfxCache.appName;
		j["Graphics"]["engineName"] = m_gfxCache.engineName;
		j["Graphics"]["appVersion"] = { m_gfxCache.appVersion.x, m_gfxCache.appVersion.y, m_gfxCache.appVersion.z };
		j["Graphics"]["imageCount"] = m_gfxCache.imageCount;
		j["Graphics"]["presentType"] = static_cast<int>(m_gfxCache.presentType);
		j["Graphics"]["rhiType"] = static_cast<int>(m_gfxCache.rhiType);

		j["Window"]["name"] = m_windowCache.name;
		j["Window"]["windowSize"] = { m_windowCache.windowSize.x, m_windowCache.windowSize.y };
		j["Window"]["mode"] = static_cast<int>(m_windowCache.mode);

		std::ofstream file(cachePath, std::ios::trunc);
		if (file.is_open())
			file << j.dump(4);
	}
}