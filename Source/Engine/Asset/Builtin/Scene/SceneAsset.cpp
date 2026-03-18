#include "SceneAsset.h"

#include <Runtime/Data/DataReader.h>

#include <fstream>

namespace Horizon
{
    SceneAsset::SceneAsset(const std::string& virtualPath, const std::filesystem::path& physicalPath) : Asset(virtualPath, physicalPath)
    {
    }

    void SceneAsset::Import()
    {
        m_parsedJson = nlohmann::json::parse(DataReader::ReadToTextFile(m_physicalPath));
        m_imported = true;
        Log::Terminal(LogType::Success, "SceneAsset", "Imported {}", m_virtualPath);
    }

    void SceneAsset::Release()
    {
        m_parsedJson.clear();
        m_imported = false;
    }

    void SceneAsset::ExportFrom(const entt::registry& registry) const
    {
        nlohmann::json root;
        root["entities"] = nlohmann::json::array();

        for (auto [entity] : registry.storage<entt::entity>()->each())
        {
            nlohmann::json entityJson;

            for (auto& [key, fn] : s_serializers)
                fn(registry, entity, entityJson);

            if (!entityJson.empty())
                root["entities"].push_back(entityJson);
        }

        std::ofstream file(m_physicalPath);
        file << root.dump(4);

        Log::Terminal(LogType::Success, "SceneAsset", "Exported {}", m_virtualPath);
    }

    void SceneAsset::ImportInto(entt::registry& registry) const
    {
        for (const auto& entityJson : m_parsedJson["entities"])
        {
            auto entity = registry.create();
            for (const auto& [key, value] : entityJson.items())
            {
                auto it = s_deserializers.find(key);
                if (it != s_deserializers.end())
                    it->second(registry, entity, value);
            }
        }
    }
}
