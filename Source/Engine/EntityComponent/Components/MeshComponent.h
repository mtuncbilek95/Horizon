#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace Horizon
{
    struct MeshComponent
    {
        std::string meshPath;
        std::string materialPath;
    };

    inline void from_json(const nlohmann::json& j, MeshComponent& m)
    {
        m.meshPath = j["meshPath"];
        m.materialPath = j["materialPath"];
    }

    inline void to_json(nlohmann::json& j, const MeshComponent& m)
    {
        j["meshPath"] = m.meshPath;
        j["materialPath"] = m.materialPath;
    }
}
