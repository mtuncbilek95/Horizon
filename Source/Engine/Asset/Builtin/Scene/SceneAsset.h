#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/EntityComponent/Components/ComponentTraits.h>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>

namespace Horizon
{
    class SceneAsset final : public Asset
    {
    public:
        using SceneComponents = std::tuple<TransformComponent, MeshComponent, CameraComponent>;

        SceneAsset(const std::string& virtualPath, const std::filesystem::path& physicalPath);
        ~SceneAsset() override = default;

        void Import() final;
        void Release() final;

        void ImportInto(entt::registry& registry) const;
        void ExportFrom(const entt::registry& registry) const;

    private:
        using DeserializeFn = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;
        using SerializeFn = std::function<void(const entt::registry&, entt::entity, nlohmann::json&)>;

        template<typename... Ts>
        static std::unordered_map<std::string, DeserializeFn> BuildDeserializers(std::tuple<Ts...>)
        {
            std::unordered_map<std::string, DeserializeFn> map;
            (map.emplace(ComponentTraits<Ts>::JsonKey, [](entt::registry& r, entt::entity e, const nlohmann::json& j)
                {
                    r.emplace<Ts>(e, j.get<Ts>());
                }), ...);
            return map;
        }

        template<typename... Ts>
        static std::unordered_map<std::string, SerializeFn> BuildSerializers(std::tuple<Ts...>)
        {
            std::unordered_map<std::string, SerializeFn> map;
            (map.emplace(ComponentTraits<Ts>::JsonKey, [](const entt::registry& r, entt::entity e, nlohmann::json& entityJson)
                {
                    if (const auto* comp = r.try_get<Ts>(e))
                        entityJson[ComponentTraits<Ts>::JsonKey] = *comp;
                }), ...);
            return map;
        }

        inline static std::unordered_map<std::string, DeserializeFn> s_deserializers =
            BuildDeserializers(SceneComponents{});

        inline static std::unordered_map<std::string, SerializeFn> s_serializers =
            BuildSerializers(SceneComponents{});

    private:
        nlohmann::json m_parsedJson;
    };
}
