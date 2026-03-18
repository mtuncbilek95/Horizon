#pragma once

#include <nlohmann/json.hpp>

namespace Horizon
{
    struct CameraComponent
    {
        float fov = 60.f;
        float nearPlane = 0.1f;
        float farPlane = 1000.f;
    };

    inline void from_json(const nlohmann::json& j, CameraComponent& c)
    {
        c.fov = j["fov"];
        c.nearPlane = j["near"];
        c.farPlane = j["far"];
    }

    inline void to_json(nlohmann::json& j, const CameraComponent& c)
    {
        j["fov"] = c.fov;
        j["near"] = c.nearPlane;
        j["far"] = c.farPlane;
    }
}
