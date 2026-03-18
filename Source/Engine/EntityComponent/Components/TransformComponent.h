#pragma once

#include <Runtime/Data/Containers/Math.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
    struct TransformComponent
    {
        Math::Vec3f position = { 0.f, 0.f, 0.f };
        Math::Quatf rotation = { 1.f, 0.f, 0.f, 0.f };
        Math::Vec3f scale = { 1.f, 1.f, 1.f };
    };

    inline void from_json(const nlohmann::json& j, TransformComponent& t)
    {
        t.position = { j["position"][0], j["position"][1], j["position"][2] };
        t.rotation = { j["rotation"][0], j["rotation"][1], j["rotation"][2], j["rotation"][3] };
        t.scale = { j["scale"][0], j["scale"][1], j["scale"][2] };
    }

    inline void to_json(nlohmann::json& j, const TransformComponent& t)
    {
        j["position"] = { t.position.x, t.position.y, t.position.z };
        j["rotation"] = { t.rotation.w, t.rotation.x, t.rotation.y, t.rotation.z };
        j["scale"] = { t.scale.x, t.scale.y, t.scale.z };
    }
}
